/* Created by ZhangZhe.*/
#include "postgres.h"
#include "utils/array.h"
#include "utils/lsyscache.h"
#include "utils/date.h"
#include "utils/datetime.h"
#include "utils/memutils.h"
#include "catalog/pg_type.h"
#include "pgtime.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/fmgrprotos.h"

#include <time.h>

PG_MODULE_MAGIC;

#define DEBUG_LOG  0

#define GET_AGG_CONTEXT(fname, fcinfo, aggcontext)  \
    if (! AggCheckCallContext(fcinfo, &aggcontext)) {   \
        elog(ERROR, "%s called in non-aggregate context", fname);  \
    }

#define CHECK_AGG_CONTEXT(fname, fcinfo)  \
    if (! AggCheckCallContext(fcinfo, NULL)) {   \
        elog(ERROR, "%s called in non-aggregate context", fname);  \
    }

#define ELEMENT_TYPE_LENGTH   sizeof(int32)
#define ELEMENT_RESULT_LENGTH sizeof(retention_result)

#define ARRAY_INIT_SIZE                 32
#define CHAR_ARRAY_INIT_SIZE            256
#define RETENTION_MAX_CACHE_SIZE        1024
#define RETENTION_MAX_DATE_INTERVAL     356
#define RETENTION_MAX_RESULT_ARRAY_SIZE 16

/*
 * Retention Input Data Format
 *   dateADT << 1 + behavior_type
 *
 * Type : int32
 * | composition  |  DateADT(31bit)  |      behavior_type(1bit)   |
 * |--------------|------------------|----------------------------|
 * | example      |  121(20000501)   |            1               |
 */
#define SET_RETENTION_DATA(dateADT, behavior_type) ((dateADT << 1) + behavior_type)
#define GET_RETENTION_DATE(data) (data >> 1)
#define GET_RETENTION_TYPE(data) (data & 1)

/* granularity behavior type */
#define RETENTION_BEHAVIOR_INITIAL  0
#define RETENTION_BEHAVIOR_RETURN   1

/* granularity type */
#define RETENTION_GRANULARITY_DAY   1
#define RETENTION_GRANULARITY_WEEK  2
#define RETENTION_GRANULARITY_MONTH 3


/* retention type */
#define RETENTION_TYPE_INITIAL_ONLY 0
#define RETENTION_TYPE_BOTH         1
#define RETENTION_TYPE_RETURN_ONLY  2

/*
 * Retention Detail Result Format
 *
 * Type : int64
 * | composition  | date(32bit) |             retention data(32bit)         |
 * |--------------|-------------|-------------------------------------------|
 * |element format| date(32bit) | retention bit(16bit) | bit length (4 bit) |
 * |--------------|-------------|----------------------|--------------------|
 * | example      |  20200501   |    100110101         |     9              |
 */

/* retention data */
#define SET_RETENTION_BIT(data, offset) \
    data = data | ( 1 << (4 + offset) )

#define SET_RETENTION_BIT_SIZE(data, size) \
    data = data + size

#define GET_RETENTION_N_BIT(data, n) ((data >> (4 + n)) & 1)
#define GET_RETENTION_SIZE(data) (data & 15)

/* retention detail result data */
#define SET_RETENTION_DETAIL_RESULT(resultData, date, retentionData) \
    resultData = ((resultData | date) << 32 ) + retentionData

#define GET_RETENTION_DETAIL_DATE(resultData)  (resultData >> 32)
#define GET_RETENTION_DETAIL_DATA(resultData) (resultData & 0x00000000FFFFFFFF)

typedef struct retention_state {
    /* retention meta info*/
    // retention granularity. day-1 week-2 month-3
    int8 granularity;

    /* array of retention data */
    Size ncapacity; // maximum capacity
    Size nlength;   // size of the data array
    int32 *data;    // dateADT << 1 + behavior_type
} retention_state;

/* only for final func */
typedef struct retention_type {
    int32 dateADT;  // date
    int8 type;      // type. -1 no event,0 initial only,1 initial and return,2 return only
} retention_type;

typedef struct retention_type_array {
    Size ncapacity;
    Size nlength;
    retention_type *data;
} retention_type_array;

typedef struct retention_detail {
    int32  dateADT;  // date
    uint32 data;     // retention bit + length
} retention_detail;

typedef struct retention_detail_array {
    Size ncapacity;
    Size nlength;
    retention_detail *data;
} retention_detail_array;

typedef struct retention_result {
    int8   length;
    int32  dateADT;     // date 20200501
    int32  size[RETENTION_MAX_RESULT_ARRAY_SIZE]; // size 100,20,10
} retention_result;

typedef struct retention_result_array {
    Size ncapacity;
    Size nlength;
    retention_result *data;
} retention_result_array;

typedef struct char_array {
    Size ncapacity;
    Size nlength;
    char *data;
} char_array;

/* transition functions */
PG_FUNCTION_INFO_V1(retention_detail_append);
PG_FUNCTION_INFO_V1(retention_result_append);
PG_FUNCTION_INFO_V1(parse_retention_data);
PG_FUNCTION_INFO_V1(retention_offset_filter);

/* parallel aggregation support functions */
PG_FUNCTION_INFO_V1(retention_detail_combine);
PG_FUNCTION_INFO_V1(retention_detail_serialize);
PG_FUNCTION_INFO_V1(retention_detail_deserialize);

PG_FUNCTION_INFO_V1(retention_result_combine);
PG_FUNCTION_INFO_V1(retention_result_serialize);
PG_FUNCTION_INFO_V1(retention_result_deserialize);

/* final functions */
PG_FUNCTION_INFO_V1(retention_detail_reducer);
PG_FUNCTION_INFO_V1(retention_result_reducer);
PG_FUNCTION_INFO_V1(retention_result_reducer_v1);

/* for retention state */
static retention_state *init(int8 granularity);
static void add(retention_state *state, int32 value);
static retention_state *copy_state(retention_state *state);
static int compare(const void *p1, const void *p2);

/* date functions */
static DateADT dateAdd(DateADT d, int delta, int8 granularity);
static int64 dateADT2date(DateADT d);
static text *retentionResult2Text(retention_result *result, char_array* buffer);

/* for retention type array */
static retention_type_array *initRetentionTypeArray();
static void appendRetentionType(retention_type_array *array, int32 dateADT, int8 type);
static void updateRetentionType(retention_type_array *array, int32 *lastADT, int32 dateADT, int8 type, int8 granularity);

/* for retention detail array */
static retention_detail_array *initRetentionDetailArray(int size);
static void appendRetentionDetail(retention_detail_array *array, int dataDAT, uint32 data);
static int compare1(const void *p1, const void *p2);

/* for retention array */
static retention_result_array *initRetentionResultArray();
static void mergeResultUnit(retention_result_array *result_array, ArrayType *array);
static void mergeResultArray(retention_result_array *larray, retention_result_array *rarray);
static void addRetentionResult(retention_result_array *array, int32 *lastADT, int32 dateADT, int offset, int32 size);
static retention_result_array *appendResultUnitV1(retention_result_array *array, retention_result* state);
static retention_result_array *appendResultUnit(retention_result_array *array, int dateADT, long retentionData);
static retention_result_array *copy_result_state(retention_result_array * state);
static int compare2(const void *p1, const void *p2);

/* for char array */
static char_array *initCharArray();
static void appendString(char_array *array, char* content,int length);


Datum retention_detail_append(PG_FUNCTION_ARGS) {

    retention_state *state;

    /* info for retention meta */
    text *dateText = PG_GETARG_TEXT_P(1);
    text *fmtText = PG_GETARG_TEXT_P(2);
    int16 behaviorType = PG_GETARG_INT16(3);
    text *granularityText = PG_GETARG_TEXT_P(4);

    DateADT dateADT = DirectFunctionCall2(to_date, PointerGetDatum(dateText), PointerGetDatum(fmtText));

    int32 data = 0;
    if (behaviorType == RETENTION_BEHAVIOR_INITIAL || behaviorType == RETENTION_BEHAVIOR_RETURN) {
        data = SET_RETENTION_DATA(dateADT, behaviorType);
    } else {
        elog(ERROR, "behaviorType=%d invalid.", behaviorType);
    }

    int8 granularitySize = VARSIZE(granularityText) - VARHDRSZ;
    char *granularityStr = text_to_cstring(granularityText);
    int8 granularity = 0;
    if (granularitySize == 3 && memcmp(granularityStr, "day", 3) == 0) {
        granularity = RETENTION_GRANULARITY_DAY;
    } else if (granularitySize == 4 && memcmp(granularityStr, "week", 4) == 0) {
        granularity = RETENTION_GRANULARITY_WEEK;
    } else if (granularitySize == 5 && memcmp(granularityStr, "month", 5) == 0) {
        granularity = RETENTION_GRANULARITY_MONTH;
    } else {
        elog(ERROR, "granularity=%s invalid.", granularityStr);
    }

#if DEBUG_LOG
    elog(INFO, "dateADT=%d,behaviorType=%d,data=%d,granularity=%d.",
         dateADT, behaviorType, data, granularity);
#endif

    /* memory contexts */
    MemoryContext oldcontext;
    MemoryContext aggcontext;

    GET_AGG_CONTEXT("retention_detail_append", fcinfo, aggcontext);

    oldcontext = MemoryContextSwitchTo(aggcontext);

    /* init the retention_state, if needed */
    if (PG_ARGISNULL(0)) {
        state = init(granularity);
    } else {
        state = (retention_state *) PG_GETARG_POINTER(0);
    }

    /* add the retention data into array */
    add(state, data);
    MemoryContextSwitchTo(oldcontext);

    PG_RETURN_POINTER(state);
}

Datum retention_detail_reducer(PG_FUNCTION_ARGS) {
#if DEBUG_LOG
    elog(INFO, "retention_detail_reducer start......");
#endif

    MemoryContext aggcontext;
    GET_AGG_CONTEXT("retention_detail_reducer", fcinfo, aggcontext);

    /* retention state */
    retention_state *state = (retention_state *) PG_GETARG_POINTER(0);
    if (state == NULL || state->nlength == 0) {
        PG_RETURN_NULL();
    }
    //sort
    qsort(state->data, state->nlength, ELEMENT_TYPE_LENGTH, compare);

    //1.retention type
    retention_type_array *typeArray = initRetentionTypeArray();
    int32 preday, day, preType, type, total = state->nlength, lastADT = -1;
    int32 i = 0, j = 1;
    while (j < total && i < total) {
        preday = GET_RETENTION_DATE(state->data[i]);
        preType = GET_RETENTION_TYPE(state->data[i]);

        day = GET_RETENTION_DATE(state->data[j]);
        type = GET_RETENTION_TYPE(state->data[j]);

        if (day == preday) {
            if (preType != type) {
                updateRetentionType(typeArray, &lastADT, preday, RETENTION_TYPE_BOTH, state->granularity);
            } else {
                elog(ERROR, "behaviorType (%d,%d,%d) invalid.", day, type, state->granularity);
            }
            i += 2;
        } else {
            if (preType == RETENTION_BEHAVIOR_INITIAL) {
                updateRetentionType(typeArray, &lastADT, preday, RETENTION_TYPE_INITIAL_ONLY, state->granularity);
            } else {
                updateRetentionType(typeArray, &lastADT, preday, RETENTION_TYPE_RETURN_ONLY, state->granularity);
            }
            i += 1;
        }

        j = i + 1;
    }

    if (i < total) {
        preday = GET_RETENTION_DATE(state->data[i]);
        preType = GET_RETENTION_TYPE(state->data[i]);

        if (preType == RETENTION_BEHAVIOR_INITIAL) {
            updateRetentionType(typeArray, &lastADT, preday, RETENTION_TYPE_INITIAL_ONLY, state->granularity);
        } else {
            updateRetentionType(typeArray, &lastADT, preday, RETENTION_TYPE_RETURN_ONLY, state->granularity);
        }
    }

#if DEBUG_LOG
    elog(INFO, "--detailReducer(retention type)--");
    for (i = 0; i < total; i++) {
        elog(INFO, "(dateADT,date,type) = %d,%ld,%d",
             retentionTypeArray->data[i].dateADT, dateADT2date(retentionTypeArray->data[i].dateADT),
             retentionTypeArray->data[i].type);
    }
    elog(INFO, "--detailReducer(retention type)--");
#endif

    //2.get result detail
    retention_detail_array* detailArray = initRetentionDetailArray(ARRAY_INIT_SIZE);
    total = typeArray->nlength;
    int32 outerIndex = 0;
    for (outerIndex = 0; outerIndex < total; outerIndex++ ) {
        int8 retentionType = typeArray->data[outerIndex].type;

        if (retentionType == RETENTION_TYPE_INITIAL_ONLY || retentionType == RETENTION_TYPE_BOTH ) {
            int32 date = typeArray->data[outerIndex].dateADT;

            int32 innerIndex = 0, size = 0, iterator = 0, offset = 0;

            uint32 retentionData = 0;
            SET_RETENTION_BIT(retentionData, offset);

            innerIndex += 1;
            size += 1;
            offset += 1;
            iterator = outerIndex + innerIndex;

            if (state->granularity == RETENTION_GRANULARITY_DAY) {
                while ((iterator < total) && (innerIndex <= 7 || innerIndex == 14 || innerIndex == 30)) {
                    retentionType = typeArray->data[iterator].type;
                    if (retentionType >= 1) {
                        SET_RETENTION_BIT(retentionData, offset);
                        size = offset + 1;
                    }
                    offset += 1;

                    if (innerIndex < 7) {
                        innerIndex = innerIndex + 1;
                    } else if (innerIndex == 7) {
                        innerIndex = 14;
                    } else if (innerIndex == 14) {
                        innerIndex = 30;
                    } else {
                        break;
                    }
                    iterator = outerIndex + innerIndex;
                }
            } else if (state->granularity == RETENTION_GRANULARITY_WEEK ||
                            state->granularity == RETENTION_GRANULARITY_MONTH) {
                while (iterator < total && innerIndex <= 9) {
                    retentionType = typeArray->data[iterator].type;
                    if (retentionType >= 1) {
                        SET_RETENTION_BIT(retentionData, offset);
                        size = offset + 1;
                    }
                    offset += 1;
                    innerIndex = innerIndex + 1;
                    iterator = outerIndex + innerIndex;
                }
            }

            SET_RETENTION_BIT_SIZE(retentionData, size);
            appendRetentionDetail(detailArray, date, retentionData);
        }
    }

#if DEBUG_LOG
    elog(INFO, "--detailReduce(result data)--");
    for (i = 0; i < detailArray->nlength; i++) {
        elog(INFO, "date=%ld", dateADT2date(detailArray->data[i].dateADT));
        uint32 retentionData = detailArray->data[i].retentionData;
        int m = 0, length = GET_RETENTION_SIZE(retentionData);
        for (m = 0; m < length; m++) {
            elog(INFO, "%d", GET_RETENTION_N_BIT(retentionData, m));
        }
    }
    elog(INFO, "--detailReduce(result data)--");
#endif

    Datum array = 0;
    if (detailArray->nlength > 0) {
        ArrayBuildState *state = NULL;
        int64 resultValue;
        for (i = 0; i < detailArray->nlength; i++) {
            resultValue = 0;
            int32 date = detailArray->data[i].dateADT;
            uint32 data = detailArray->data[i].data;
            SET_RETENTION_DETAIL_RESULT(resultValue, date, data);

            state = accumArrayResult(state, Int64GetDatum(resultValue), false, INT8OID, aggcontext);
        }
        array = makeArrayResult(state, CurrentMemoryContext);
    }

    //free
    pfree(typeArray->data);
    pfree(typeArray);

    pfree(detailArray->data);
    pfree(detailArray);

    if (array == 0) {
        PG_RETURN_NULL();
    }

    PG_RETURN_ARRAYTYPE_P(array);
}

Datum retention_result_reducer(PG_FUNCTION_ARGS) {
#if DEBUG_LOG
    elog(INFO, "retention_result_reducer start......");
#endif

    ArrayType *array;
    Oid arrayElementType;
    int16 arrayElementTypeWidth;
    bool arrayElementTypeByValue;
    char arrayElementTypeAlignmentCode;

    Datum *arrayContent;
    bool *arrayNullFlags;
    int arrayLength;

    MemoryContext aggcontext;
    GET_AGG_CONTEXT("retention_result_reducer", fcinfo, aggcontext);

    if (PG_ARGISNULL(0)) {
        PG_RETURN_NULL();
    }

    array = PG_GETARG_ARRAYTYPE_P(0);
    arrayElementType = ARR_ELEMTYPE(array);
    get_typlenbyvalalign(arrayElementType, &arrayElementTypeWidth, &arrayElementTypeByValue,
                         &arrayElementTypeAlignmentCode);
    deconstruct_array(array, arrayElementType, arrayElementTypeWidth, arrayElementTypeByValue,
                      arrayElementTypeAlignmentCode,
                      &arrayContent, &arrayNullFlags, &arrayLength);
    retention_detail *detailArray = (retention_detail *) palloc(arrayLength * sizeof(retention_detail));

    if (arrayLength == 0) {
        PG_RETURN_NULL();
    }

    int i, j, size;
    for (i = 0; i < arrayLength; i++) {
        int64 value = DatumGetInt64(arrayContent[i]);
        detailArray[i].dateADT = GET_RETENTION_DETAIL_DATE(value);
        detailArray[i].data = GET_RETENTION_DETAIL_DATA(value);
    }
    qsort(detailArray, arrayLength, sizeof(retention_detail), compare1);

    retention_result_array *result_array = initRetentionResultArray();
    int32 lastADT = -1;
    for (i = 0; i < arrayLength; i++) {
        int32 date = detailArray[i].dateADT;
        uint32 data = detailArray[i].data;

        size = GET_RETENTION_SIZE(data);
        for (j = 0; j < size; j++) {
            int8 bit = GET_RETENTION_N_BIT(data, j);
            addRetentionResult(result_array, &lastADT, date, j, bit);
        }
    }

    ArrayType* resultArray = NULL;
    if ( result_array->nlength > 0) {
        Datum *tmpArray = (Datum *) palloc(sizeof(Datum) * result_array->nlength);
        char_array *charArray = initCharArray();

        for (i = 0; i < result_array->nlength; i++) {
            retention_result *p = result_array->data + i;
            text *text = retentionResult2Text(p, charArray);
            tmpArray[i] = PointerGetDatum(text);
        }

        get_typlenbyvalalign(TEXTOID, &arrayElementTypeWidth, &arrayElementTypeByValue, &arrayElementTypeAlignmentCode);
        resultArray = construct_array(tmpArray, result_array->nlength, TEXTOID,
                arrayElementTypeWidth, arrayElementTypeByValue, arrayElementTypeAlignmentCode);

        // Return the final PostgreSQL array object.
        pfree(charArray->data);
        pfree(charArray);
    }

    pfree(detailArray);
    pfree(result_array->data);
    pfree(result_array);

    if (resultArray == NULL) {
        PG_RETURN_NULL();
    }

    PG_RETURN_ARRAYTYPE_P(resultArray);
}

Datum retention_detail_serialize(PG_FUNCTION_ARGS) {
#if DEBUG_LOG
    elog(INFO, "retention_detail_serialize ......");
#endif

    retention_state *state = (retention_state *) PG_GETARG_POINTER(0);
    Assert(state != NULL);

    CHECK_AGG_CONTEXT("retention_detail_serialize", fcinfo);

    bytea *out;
    char *ptr;

    int hlen = offsetof(retention_state, data);
    int dlen = state->nlength * ELEMENT_TYPE_LENGTH;

    out = (bytea *) palloc(VARHDRSZ + dlen + hlen);
    SET_VARSIZE(out, VARHDRSZ + dlen + hlen);
    ptr = VARDATA(out);

    memcpy(ptr, state, hlen);
    memcpy(ptr + hlen, state->data, dlen);


    PG_RETURN_BYTEA_P(out);
}

Datum retention_detail_deserialize(PG_FUNCTION_ARGS) {
#if DEBUG_LOG
    elog(INFO, "retention_detail_deserialize ......");
#endif

    bytea *bytes = (bytea *) PG_GETARG_POINTER(0);
    retention_state *state = (retention_state *) palloc(sizeof(retention_state));

    CHECK_AGG_CONTEXT("retention_detail_deserialize", fcinfo);

    char *ptr = VARDATA_ANY(bytes);

    memcpy(state, ptr, offsetof(retention_state, data));
    state->data = (int32 *) palloc(state->nlength * ELEMENT_TYPE_LENGTH);
    memcpy(state->data, ptr + offsetof(retention_state, data), state->nlength * ELEMENT_TYPE_LENGTH);

    PG_RETURN_POINTER(state);
}

Datum retention_detail_combine(PG_FUNCTION_ARGS) {
#if DEBUG_LOG
    elog(INFO, "retention_detail_combine ......");
#endif

    retention_state *state1;
    retention_state *state2;

    MemoryContext agg_context;
    MemoryContext old_context;

    GET_AGG_CONTEXT("retention_detail_combine", fcinfo, agg_context);

    state1 = PG_ARGISNULL(0) ? NULL : (retention_state *) PG_GETARG_POINTER(0);
    state2 = PG_ARGISNULL(1) ? NULL : (retention_state *) PG_GETARG_POINTER(1);

    if (state2 == NULL) {
        PG_RETURN_POINTER(state1);
    }

    if (state1 == NULL) {
        old_context = MemoryContextSwitchTo(agg_context);

        state1 = copy_state(state2);
        MemoryContextSwitchTo(old_context);

        PG_RETURN_POINTER(state1);
    }

    int i;
    for (i = 0; i < state2->nlength; i++) {
        add(state1, state2->data[i]);
    }

    pfree(state2->data);

    PG_RETURN_POINTER(state1);
}

Datum retention_offset_filter(PG_FUNCTION_ARGS) {

    int8 ret = 0;

    //check args
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
        PG_RETURN_INT16(ret);
    }

    int16 offset = PG_GETARG_INT16(1);
    if (offset < 0) {
        PG_RETURN_INT16(ret);
    }

    ArrayType *array;
    int16 arrayElementTypeWidth;
    bool arrayElementTypeByValue;
    char arrayElementTypeAlignmentCode;

    Datum *arrayContent;
    bool *arrayNullFlags;
    int arrayLength;

    array = PG_GETARG_ARRAYTYPE_P(0);
    get_typlenbyvalalign(INT8OID, &arrayElementTypeWidth, &arrayElementTypeByValue,
                         &arrayElementTypeAlignmentCode);
    deconstruct_array(array, INT8OID, arrayElementTypeWidth, arrayElementTypeByValue,
                      arrayElementTypeAlignmentCode, &arrayContent, &arrayNullFlags, &arrayLength);
    int i = 0;
    for (i = 0; i < arrayLength; i++) {
        int64 value = DatumGetInt64(arrayContent[i]);
        int32 data = GET_RETENTION_DETAIL_DATA(value);

        if (GET_RETENTION_SIZE(data) > offset && GET_RETENTION_N_BIT(data, offset) == 1) {
            ret = 1;
            break;
        }
    }

    PG_RETURN_INT16(ret);
}


Datum parse_retention_data(PG_FUNCTION_ARGS) {

    ArrayType *array;
    int16 arrayElementTypeWidth;
    bool arrayElementTypeByValue;
    char arrayElementTypeAlignmentCode;

    Datum *arrayContent;
    bool *arrayNullFlags;
    int arrayLength;

    if (PG_ARGISNULL(0)) {
        PG_RETURN_NULL();
    }
    array = PG_GETARG_ARRAYTYPE_P(0);
    get_typlenbyvalalign(INT8OID, &arrayElementTypeWidth, &arrayElementTypeByValue,
                         &arrayElementTypeAlignmentCode);

    deconstruct_array(array, INT8OID, arrayElementTypeWidth, arrayElementTypeByValue,
            arrayElementTypeAlignmentCode, &arrayContent, &arrayNullFlags, &arrayLength);

    int i = 0,j = 0,size = 0;
    char tmp[10];

    char_array* buffer = initCharArray();
    for (i = 0; i < arrayLength; i++) {
        int64 value = DatumGetInt64(arrayContent[i]);
        int32 date = GET_RETENTION_DETAIL_DATE(value);
        int32 data = GET_RETENTION_DETAIL_DATA(value);
        size = GET_RETENTION_SIZE(data);

        memset(tmp, 0, 10);
        if (buffer->nlength > 0) {
            tmp[0] = ',';
            appendString(buffer, tmp, 1);
        }

        //date parser
        j = snprintf(tmp, 10, "%ld", dateADT2date(date));
        appendString(buffer, tmp, j);

        tmp[0] = ':';
        appendString(buffer, tmp, 1);

        //retention bit
        for (j = 0 ; j < size; j++) {
            tmp[j] = GET_RETENTION_N_BIT(data, j) + '0';
        }
        appendString(buffer, tmp, size);
    }

    text *result = cstring_to_text_with_len(buffer->data, buffer->nlength);

    //free buffer
    pfree(buffer->data);
    pfree(buffer);

    PG_RETURN_TEXT_P(result);
}

Datum retention_result_append(PG_FUNCTION_ARGS) {
#if DEBUG_LOG
    elog(INFO, "retention_result_append start......");
#endif

    /* memory contexts */
    MemoryContext oldcontext;
    MemoryContext aggcontext;

    GET_AGG_CONTEXT("retention_result_append", fcinfo, aggcontext);

    retention_result_array *state;

    oldcontext = MemoryContextSwitchTo(aggcontext);
    if (PG_ARGISNULL(0)) {
        state = initRetentionResultArray();
    } else {
        state = (retention_result_array*) PG_GETARG_ARRAYTYPE_P(0);
    }

    if (!PG_ARGISNULL(1)) {
        mergeResultUnit(state, PG_GETARG_ARRAYTYPE_P(1));
    }

    MemoryContextSwitchTo(oldcontext);

#if DEBUG_LOG
    elog(INFO, "retention_result_append end......");
#endif

    PG_RETURN_POINTER(state);
}

Datum retention_result_combine(PG_FUNCTION_ARGS) {
#if DEBUG_LOG
    elog(INFO, "retention_result_combine ......");
#endif

    retention_result_array *state1;
    retention_result_array *state2;

    MemoryContext agg_context;
    MemoryContext old_context;

    GET_AGG_CONTEXT("retention_result_combine", fcinfo, agg_context);

    state1 = PG_ARGISNULL(0) ? NULL : (retention_result_array *) PG_GETARG_POINTER(0);
    state2 = PG_ARGISNULL(1) ? NULL : (retention_result_array *) PG_GETARG_POINTER(1);

    if (state2 == NULL) {
        PG_RETURN_POINTER(state1);
    }

    if (state1 == NULL) {
        old_context = MemoryContextSwitchTo(agg_context);

        state1 = copy_result_state(state2);
        MemoryContextSwitchTo(old_context);

        PG_RETURN_POINTER(state1);
    }

    mergeResultArray(state1, state2);

    pfree(state2->data);

    PG_RETURN_POINTER(state1);
}


Datum retention_result_serialize(PG_FUNCTION_ARGS) {
#if DEBUG_LOG
    elog(INFO, "retention_result_serialize ......");
#endif

    retention_result_array *state = (retention_result_array *) PG_GETARG_POINTER(0);
    Assert(state != NULL);

    CHECK_AGG_CONTEXT("retention_result_serialize", fcinfo);

    bytea *out;
    char *ptr;

    int hlen = offsetof(retention_result_array, data);
    int dlen = state->nlength * ELEMENT_RESULT_LENGTH;

    out = (bytea *) palloc(VARHDRSZ + dlen + hlen);
    SET_VARSIZE(out, VARHDRSZ + dlen + hlen);
    ptr = VARDATA(out);

    memcpy(ptr, state, hlen);
    memcpy(ptr + hlen, state->data, dlen);

    PG_RETURN_BYTEA_P(out);

}

Datum retention_result_deserialize(PG_FUNCTION_ARGS) {
#if DEBUG_LOG
    elog(INFO, "retention_result_deserialize ......");
#endif

    bytea *bytes = (bytea *) PG_GETARG_POINTER(0);
    retention_result_array *state = (retention_result_array *) palloc(sizeof(retention_result_array));

    CHECK_AGG_CONTEXT("retention_result_deserialize", fcinfo);

    char *ptr = VARDATA_ANY(bytes);

    memcpy(state, ptr, offsetof(retention_result_array, data));
    state->data = (retention_result *) palloc(state->nlength * ELEMENT_RESULT_LENGTH);
    memcpy(state->data, ptr + offsetof(retention_result_array, data), state->nlength * ELEMENT_RESULT_LENGTH);

    PG_RETURN_POINTER(state);
}

Datum retention_result_reducer_v1(PG_FUNCTION_ARGS) {
#if DEBUG_LOG
    elog(INFO, "retention_result_reducer_v1 ......");
#endif

    CHECK_AGG_CONTEXT("retention_result_reducer_v1", fcinfo);

    if (PG_ARGISNULL(0)) {
        PG_RETURN_NULL();
    }

    /* retention_result_array */
    retention_result_array *result_array = (retention_result_array *) PG_GETARG_POINTER(0);
    if (result_array == NULL || result_array->nlength == 0) {
        PG_RETURN_NULL();
    }

    ArrayType *resultArray = NULL;
    int16 arrayElementTypeWidth;
    bool arrayElementTypeByValue;
    char arrayElementTypeAlignmentCode;
    int i = 0;

    Datum *tmpArray = (Datum *) palloc(sizeof(Datum) * result_array->nlength);
    char_array *buffer = initCharArray();
    for (i = 0; i < result_array->nlength; i++) {
        text *text = retentionResult2Text(result_array->data + i, buffer);
        tmpArray[i] = PointerGetDatum(text);
    }

    get_typlenbyvalalign(TEXTOID, &arrayElementTypeWidth, &arrayElementTypeByValue, &arrayElementTypeAlignmentCode);
    resultArray = construct_array(tmpArray, result_array->nlength, TEXTOID,
                                  arrayElementTypeWidth, arrayElementTypeByValue, arrayElementTypeAlignmentCode);
    pfree(buffer->data);
    pfree(buffer);
    pfree(tmpArray);

    if (resultArray == NULL) {
        PG_RETURN_NULL();
    }

    PG_RETURN_ARRAYTYPE_P(resultArray);
}

static retention_result_array *appendResultUnit(retention_result_array *array, int dateADT, long data) {
    if (array->nlength + 1 > array->ncapacity) {
        array->ncapacity *= 2;
        retention_result *ptr = (retention_result *) repalloc(array->data,
                sizeof(retention_result) * array->ncapacity);
        if (ptr != NULL) {
            array->data = ptr;
        }
    }

    //init
    retention_result *result = array->data + array->nlength ;
    result->dateADT = dateADT;
    result->length = 0;
    int i = 0;
    for (i = 0; i < RETENTION_MAX_RESULT_ARRAY_SIZE; i++) {
        result->size[i] = 0;
    }

    //append
    int size = GET_RETENTION_SIZE(data);
    for (i = 0; i < size; i++) {
        result->size[i] = GET_RETENTION_N_BIT(data, i);
    }
    result->length += size;
    array->nlength += 1;

    return array;
}

static retention_result_array *appendResultUnitV1(retention_result_array *array, retention_result* data) {
    if (array->nlength + 1 > array->ncapacity) {
        array->ncapacity *= 2;
        retention_result *ptr = (retention_result *) repalloc(array->data, sizeof(retention_result) * array->ncapacity);
        if (ptr != NULL) {
            array->data = ptr;
        }
    }

    //init
    retention_result *result = array->data + array->nlength ;
    result->dateADT = data->dateADT;
    result->length = 0;
    int i = 0;
    for (i = 0; i < RETENTION_MAX_RESULT_ARRAY_SIZE; i++) {
        result->size[i] = 0;
    }

    //append
    int size = data->length;
    for (i = 0; i < size; i++) {
        result->size[i] = data->size[i];
    }
    result->length += size;
    array->nlength += 1;

    return array;
}


static void mergeResultUnit(retention_result_array *result_array, ArrayType *array) {

    Oid arrayElementType;
    int16 arrayElementTypeWidth;
    bool arrayElementTypeByValue;
    char arrayElementTypeAlignmentCode;

    Datum *arrayContent;
    bool *arrayNullFlags;
    int arrayLength;

    arrayElementType = ARR_ELEMTYPE(array);
    get_typlenbyvalalign(arrayElementType, &arrayElementTypeWidth, &arrayElementTypeByValue,
                         &arrayElementTypeAlignmentCode);
    deconstruct_array(array, arrayElementType, arrayElementTypeWidth, arrayElementTypeByValue,
                      arrayElementTypeAlignmentCode,
                      &arrayContent, &arrayNullFlags, &arrayLength);

    if (arrayLength == 0) {
        return;
    }

    int i = 0, j = 0, m = 0, flag = 0, lsize = 0, rsize = 0, rtotal, ltotal;
    int ldate, rdate, data;

    //init size
    ltotal = result_array->nlength;
    rtotal = arrayLength;

    i = 0, j = 0;
    while (i < ltotal && j < rtotal) {
        ldate = result_array->data[i].dateADT;

        //提取第j个元素
        int64 retentionData = DatumGetInt64(arrayContent[j]);
        rdate = GET_RETENTION_DETAIL_DATE(retentionData);
        data = GET_RETENTION_DETAIL_DATA(retentionData);

        //直接进行merge
        if (ldate == rdate) {
            lsize = result_array->data[i].length;
            rsize = GET_RETENTION_SIZE(data);

            for (m = 0; m < lsize && m < rsize; m++) {
                result_array->data[i].size[m] += GET_RETENTION_N_BIT(data, m);
            }

            for (;  m < rsize; m++) {
                result_array->data[i].size[m] += GET_RETENTION_N_BIT(data, m);
                result_array->data[i].length++;
            }
            i++;
            j++;
        } else if (ldate > rdate) {
            //追加  date到  retention_result_array
            result_array = appendResultUnit(result_array, rdate, data);
            flag = 1;
            j++;
        } else if (ldate < rdate) {
            i++;
        }
    }

    while (j < rtotal) {
        //追加  date 到  retention_result_array
        int64 data = DatumGetInt64(arrayContent[j]);
        result_array = appendResultUnit(result_array,GET_RETENTION_DETAIL_DATE(data), GET_RETENTION_DETAIL_DATA(data));
        flag = 1;
        j++;
    }

    if (flag == 1 && result_array->nlength > 1) {
        //qsort 保持全局有序
        qsort(result_array->data, result_array->nlength, ELEMENT_RESULT_LENGTH, compare2);
    }
}

static void mergeResultArray(retention_result_array *larray, retention_result_array *rarray) {
    int i = 0, j = 0, m = 0, flag = 0, lsize = 0, rsize = 0, ltotal = 0, rtotal = 0, ldate, rdate;

    //init size
    ltotal = larray->nlength;
    rtotal = rarray->nlength;

    i = 0, j = 0;
    while (i < ltotal && j < rtotal) {
        ldate = larray->data[i].dateADT;
        rdate = rarray->data[j].dateADT;

        //直接进行merge
        if (ldate == rdate) {
            lsize = larray->data[i].length;
            rsize = rarray->data[j].length;

            for (m = 0; m < lsize && m < rsize; m++) {
                larray->data[i].size[m] += rarray->data[j].size[m];
            }

            for (; m < rsize; m++) {
                larray->data[i].size[m] = rarray->data[j].size[m];
                larray->data[i].length++;
            }
            i++;
            j++;
        } else if (ldate > rdate) {
            larray = appendResultUnitV1(larray, rarray->data + j);
            flag = 1;
            j++;
        } else if (ldate < rdate) {
            i++;
        }
    }

    while (j < rtotal) {
        larray = appendResultUnitV1(larray, rarray->data + j);
        flag = 1;
        j++;
    }

    if (flag == 1 && larray->nlength > 1) {
        qsort(larray->data, larray->nlength, ELEMENT_RESULT_LENGTH, compare2);
    }
}

static retention_state *init(int8 granularity) {
    retention_state *p = (retention_state *) palloc(sizeof(retention_state));
    p->granularity = granularity;
    p->ncapacity = ARRAY_INIT_SIZE;
    p->nlength = 0;
    p->data = (int32 *) palloc(p->ncapacity * ELEMENT_TYPE_LENGTH);

    return p;
}

static void add(retention_state *state, int32 value) {
    if (state->nlength > RETENTION_MAX_CACHE_SIZE) {
        elog(WARNING, "retention data size (%lu) larger than %d.", state->nlength, RETENTION_MAX_CACHE_SIZE);
        return;
    }

    if (state->nlength + 1 > state->ncapacity) {
        if (state->ncapacity * ELEMENT_TYPE_LENGTH / 0.8 < ALLOCSET_SEPARATE_THRESHOLD) {
            state->ncapacity *= 2;
        } else {
            state->ncapacity /= 0.8;
        }

        int32* ptr = (int32 *)repalloc(state->data, state->ncapacity * ELEMENT_TYPE_LENGTH);
        if (ptr != NULL) {
            state->data = ptr;
        } else {
            elog(ERROR, "retention_state repalloc failed.");
        }
    }

    state->data[state->nlength] = value;
    state->nlength += 1;
}

static retention_state *copy_state(retention_state *state) {
    retention_state *copy = (retention_state *) palloc(sizeof(retention_state));
    copy->granularity = state->granularity;

    copy->nlength = state->nlength;
    copy->ncapacity = state->ncapacity;

    copy->data = palloc(copy->ncapacity * ELEMENT_TYPE_LENGTH);
    memcpy(copy->data, state->data, state->nlength * ELEMENT_TYPE_LENGTH);

    return copy;
}

static retention_result_array *copy_result_state(retention_result_array * state) {
    retention_result_array *copy = (retention_result_array *) palloc(sizeof(retention_result_array));
    copy->nlength = state->nlength;
    copy->ncapacity = state->ncapacity;

    copy->data = palloc(copy->ncapacity * ELEMENT_RESULT_LENGTH);
    memcpy(copy->data, state->data, state->nlength * ELEMENT_RESULT_LENGTH);

    return copy;
}

static int compare(const void *p1, const void *p2) {
    if(GET_RETENTION_DATE(*(int32 *) p1) > GET_RETENTION_DATE(*(int32 *) p2)) {
        return 1;
    } else if (GET_RETENTION_DATE(*(int32 *) p1) < GET_RETENTION_DATE(*(int32 *) p2)) {
        return -1;
    } else {
        return GET_RETENTION_TYPE(*(int32 *) p1) - GET_RETENTION_TYPE(*(int32 *) p2);
    }
}

static int compare1(const void *p1, const void *p2) {
    if(((retention_detail*)p1)->dateADT > ((retention_detail*)p2)->dateADT) {
        return 1;
    } else if (((retention_detail*)p1)->dateADT < ((retention_detail*)p2)->dateADT) {
        return -1;
    } else {
        return 0;
    }
}

static int compare2(const void *p1, const void *p2) {
    if((*(retention_result *) p1).dateADT > (*(retention_result *) p2).dateADT) {
        return 1;
    } else if ((*(retention_result *) p1).dateADT < (*(retention_result *) p2).dateADT) {
        return -1;
    } else {
        return 0;
    }
}

static retention_type_array *initRetentionTypeArray() {
    retention_type_array *array = (retention_type_array *) palloc(sizeof(retention_type_array));
    array->ncapacity = ARRAY_INIT_SIZE;
    array->nlength = 0;
    array->data = (retention_type *) palloc(sizeof(retention_type) * array->ncapacity);

    return array;
}

static void updateRetentionType(retention_type_array *array, int32 *lastADT, int32 dateADT, int8 type, int8 granularity) {
    if (*lastADT != -1) {
        int i = 0;
        while (i++ < RETENTION_MAX_DATE_INTERVAL) {
            int32 next = dateAdd((*lastADT), 1, granularity);
            if (next != dateADT) {
                appendRetentionType(array, next, -1);
                (*lastADT) = next;
            } else {
                break;
            }
        }

        if (i == RETENTION_MAX_DATE_INTERVAL) {
            elog(ERROR, "retention max interval is greater than %d.", RETENTION_MAX_DATE_INTERVAL);
        }
    }

    appendRetentionType(array, dateADT, type);
    (*lastADT) = dateADT;
}

static void appendRetentionType(retention_type_array *array, int32 dateADT, int8 type) {
    if (array->nlength + 1 > array->ncapacity) {
        array->ncapacity *= 2;

        retention_type *ptr = (retention_type *) repalloc(array->data, sizeof(retention_type) * array->ncapacity);
        if (ptr != NULL) {
            array->data = ptr;
        } else {
            elog(ERROR, "retention_type_array repalloc failed.");
        }
    }

    array->data[array->nlength].dateADT = dateADT;
    array->data[array->nlength].type = type;
    array->nlength += 1;
}

static retention_detail_array *initRetentionDetailArray(int size) {
    retention_detail_array *array = (retention_detail_array *) palloc(sizeof(retention_detail_array));
    array->ncapacity = size;
    array->nlength = 0;
    array->data = (retention_detail *) palloc(sizeof(retention_detail) * array->ncapacity);

    return array;
}

static void appendRetentionDetail(retention_detail_array *array, int dateADT, uint32 data) {
    if (array->nlength + 1 > array->ncapacity) {
        array->ncapacity *= 2;
        retention_detail *ptr = (retention_detail *) repalloc(array->data, sizeof(retention_detail) * array->ncapacity);
        if (ptr != NULL) {
            array->data = ptr;
        } else {
            elog(ERROR, "retention_detail_array repalloc failed.");
        }
    }

    array->data[array->nlength].dateADT= dateADT;
    array->data[array->nlength].data = data;
    array->nlength += 1;
}

static int64 dateADT2date(DateADT d) {
    DateADT inner = d + POSTGRES_EPOCH_JDATE;
    struct pg_tm pg;
    j2date(inner, &pg.tm_year, &pg.tm_mon, &pg.tm_mday);

    return pg.tm_year * 10000 + pg.tm_mon * 100 + pg.tm_mday;
}

static DateADT dateAdd(DateADT d, int delta, int8 granularity) {
    Interval spanTime;
    spanTime.time = 0;
    spanTime.day = 0;
    spanTime.month = 0;

    if (granularity == 1) {
        spanTime.day = delta;
    } else if (granularity == 2) {
        spanTime.day = delta * 7;
    } else  {
        spanTime.month = delta;
    }

    Timestamp ts = DirectFunctionCall2(date_pl_interval, DateADTGetDatum(d), PointerGetDatum(&spanTime));
    struct pg_tm tt;
    fsec_t fsec;
    int tz;
    if (timestamp2tm(ts, &tz, &tt, &fsec, NULL, NULL) != 0)
        ereport(ERROR,
                (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
                        errmsg("timestamp out of range")));

    int32 result = date2j(tt.tm_year, tt.tm_mon, tt.tm_mday) - POSTGRES_EPOCH_JDATE;

#if DEBUG_LOG
    elog(INFO, "--dateAdd(origin date=%d,delta=%d,granularity=%d,result=%d.)--", d, delta, granularity, result);
#endif

    return result;
}

static char_array *initCharArray() {
    char_array *array = (char_array *) palloc(sizeof(char_array));
    array->ncapacity = CHAR_ARRAY_INIT_SIZE;
    array->nlength = 0;
    array->data = (char *) palloc(sizeof(char) * array->ncapacity);
    memset(array->data, 0, array->ncapacity);

    return array;
}

static void appendString(char_array *array, char *content, int length) {
    if (array->nlength + length > array->ncapacity) {
        array->ncapacity *= 2;
        char *ptr = (char *) repalloc(array->data, sizeof(char) * array->ncapacity);
        if (ptr != NULL) {
            array->data = ptr;
        } else {
            elog(ERROR, "char array repalloc failed.");
        }
    }

    strncpy(array->data + array->nlength, content, length);
    array->nlength += length;
}

static retention_result_array *initRetentionResultArray() {
    retention_result_array *array = (retention_result_array *) palloc(sizeof(retention_result_array));
    array->ncapacity = ARRAY_INIT_SIZE;
    array->nlength = 0;
    array->data = (retention_result *) palloc(sizeof(retention_result) * array->ncapacity);

    return array;
}

static void addRetentionResult(retention_result_array *array, int32 *lastADT, int32 dateADT, int offset, int32 size) {
    if (*lastADT != dateADT) {
        if (array->nlength + 1 > array->ncapacity) {
            array->ncapacity *= 2;
            retention_result *ptr = (retention_result *) repalloc(array->data,
                                                                  sizeof(retention_result) * array->ncapacity);
            if (ptr != NULL) {
                array->data = ptr;
            } else {
                elog(ERROR, "retention_result_array repalloc failed.");
            }
        }

        //init retention_result
        retention_result *p = array->data + array->nlength;
        p->length = 0;
        int i = 0;
        for (i = 0; i < RETENTION_MAX_RESULT_ARRAY_SIZE; i++) {
            p->size[i] = 0;
        }
        p->dateADT = dateADT;

        array->nlength += 1;
        (*lastADT) = dateADT;
    }

    retention_result *result = array->data + array->nlength - 1;
    result->size[offset] = result->size[offset] + size;
    if (offset + 1 > result->length) {
        result->length = offset + 1;
    }
}

static text *retentionResult2Text(retention_result *result, char_array* buffer) {
    memset(buffer->data, 0, buffer->ncapacity);
    buffer->nlength = 0;

    char tmp[20];
    int i, j, value;

    //date parser
    j = snprintf(tmp, 20, "%ld", dateADT2date(result->dateADT));
    appendString(buffer, tmp, j);

    tmp[0]=':';
    appendString(buffer, tmp, 1);

    //size parser
    for (i = 0; i < result->length; i++) {
        if (i > 0) {
            tmp[0] = ',';
            appendString(buffer, tmp, 1);
        }

        value = result->size[i];
        j = snprintf(tmp, 20, "%d", value);
        appendString(buffer, tmp, j);
    }

    return cstring_to_text_with_len(buffer->data, buffer->nlength);
}