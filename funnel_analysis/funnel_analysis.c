/* Created by ZhangZhe.*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "postgres.h"
#include "utils/array.h"
#include "utils/lsyscache.h"
#include "utils/memutils.h"
#include "catalog/pg_type.h"

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

#define ELEMENT_TYPE_LENGTH sizeof(int64)
#define ARRAY_INIT_SIZE  32
#define FUNNEL_MAX_STEP  10
#define FUNNEL_MAX_DATA_SIZE 10240

#define GET_FUNNEL_DATA(time, step) ((time << 4) + step)
#define GET_FUNNEL_TIME(data) (data >> 4)
#define GET_FUNNEL_STEP(data) (data & 15)


typedef struct funnel_state {
    /* aggregation memory context (don't need to do lookups repeatedly) */
    MemoryContext aggctx;

    /* funnel meta info */
    int8 totalStep;   // funnel total steps
    int64 windowSize; // funnel window size (ms)
    int16 nfirst;     // number of first step

    /* array of funnel data */
    Size ncapacity;   // maximum capacity
    Size nlength;     // size of the data array
    int64 *data;      // client_timestamp << 4 + step
} funnel_state;


/* only for final func */
typedef struct funnel_unit {
    int64 startTime;       // first step timestamp
    int8 currentStep;     // current max step
    int64 currentStepTime; // current step timestamp
} funnel_unit;


/* transition functions */
PG_FUNCTION_INFO_V1(funnel_detail_append);

/* parallel aggregation support functions */
PG_FUNCTION_INFO_V1(funnel_detail_combine);
PG_FUNCTION_INFO_V1(funnel_detail_serialize);
PG_FUNCTION_INFO_V1(funnel_detail_deserialize);

/* final functions */
PG_FUNCTION_INFO_V1(funnel_detail_reducer);
PG_FUNCTION_INFO_V1(funnel_result_reducer);

static int compare(const void *p1, const void *p2);
static funnel_state *init(int8 totalStep, int64 windowSize, MemoryContext ctx);
static void add(funnel_state *state, int64 value);
static int16 detailReduce(funnel_state *state);
static funnel_state *copy_state(funnel_state *state);

Datum funnel_detail_append(PG_FUNCTION_ARGS) {
#if DEBUG_LOG
    elog(INFO, "funnel_detail_append start......");
#endif

    funnel_state *funnelState;

    /* info for funnel detail */
    int32 totalStep = PG_GETARG_INT32(1);
    int64 windowSize = PG_GETARG_INT64(2);
    int64 eventTime = PG_GETARG_INT64(3);
    int8  currentStep = PG_GETARG_INT64(4);

    /* memory contexts */
    MemoryContext oldcontext;
    MemoryContext aggcontext;

    /* switch to the per-group  memory context */
    GET_AGG_CONTEXT("funnel_detail_append", fcinfo, aggcontext);

    oldcontext = MemoryContextSwitchTo(aggcontext);

    /* init the funnelState, if needed */
    if (PG_ARGISNULL(0)) {
        funnelState = init(totalStep, windowSize, aggcontext);
    } else {
        funnelState = (funnel_state *) PG_GETARG_POINTER(0);
    }

    /* add the funnel data into array */
    add(funnelState, GET_FUNNEL_DATA(eventTime, currentStep));

    MemoryContextSwitchTo(oldcontext);

#if DEBUG_LOG
    elog(INFO, "funnel_detail_append end......");
#endif

    PG_RETURN_POINTER(funnelState);
}


Datum funnel_detail_reducer(PG_FUNCTION_ARGS) {
#if DEBUG_LOG
    elog(INFO, "funnel_detail_reducer end......");
#endif

    funnel_state *funnelState;

    CHECK_AGG_CONTEXT("funnel_detail_reducer", fcinfo);

    /* funnel state */
    funnelState = (funnel_state *) PG_GETARG_POINTER(0);

    /* funnel detail reducer  */
    int8 ret = detailReduce(funnelState);

#if DEBUG_LOG
    elog(INFO, "funnel_detail_reducer end......");
#endif

    PG_RETURN_INT16(ret);
}

Datum funnel_detail_serialize(PG_FUNCTION_ARGS) {
#if DEBUG_LOG
    elog(INFO, "funnel_detail_serialize ......");
#endif
    funnel_state *state = (funnel_state *) PG_GETARG_POINTER(0);
    Assert(state != NULL);

    CHECK_AGG_CONTEXT("funnel_detail_serialize", fcinfo);

    bytea *out;
    char *ptr;

    int hlen = offsetof(funnel_state, data);
    int dlen = state->nlength * ELEMENT_TYPE_LENGTH;

    out = (bytea *) palloc(VARHDRSZ + dlen + hlen);
    SET_VARSIZE(out, VARHDRSZ + dlen + hlen);
    ptr = VARDATA(out);

    memcpy(ptr, state, hlen);
    memcpy(ptr + hlen, state->data, dlen);


    PG_RETURN_BYTEA_P(out);
}

Datum funnel_detail_deserialize(PG_FUNCTION_ARGS) {
#if DEBUG_LOG
    elog(INFO, "funnel_detail_deserialize ......");
#endif
    bytea *bytes = (bytea *) PG_GETARG_POINTER(0);
    funnel_state *state = (funnel_state *) palloc(sizeof(funnel_state));

    CHECK_AGG_CONTEXT("funnel_detail_deserialize", fcinfo);

    char *ptr = VARDATA_ANY(bytes);

    memcpy(state, ptr, offsetof(funnel_state, data));
    state->data = (int64 *) palloc(state->nlength * ELEMENT_TYPE_LENGTH);
    memcpy(state->data, ptr + offsetof(funnel_state, data), state->nlength * ELEMENT_TYPE_LENGTH);
    PG_RETURN_POINTER(state);
}

Datum funnel_detail_combine(PG_FUNCTION_ARGS) {
#if DEBUG_LOG
    elog(INFO, "funnel_detail_combine ......");
#endif

    funnel_state *state1;
    funnel_state *state2;

    MemoryContext	agg_context;
    MemoryContext	old_context;

    GET_AGG_CONTEXT("funnel_detail_combine", fcinfo, agg_context);

    state1 = PG_ARGISNULL(0) ? NULL : (funnel_state *) PG_GETARG_POINTER(0);
    state2 = PG_ARGISNULL(1) ? NULL : (funnel_state *) PG_GETARG_POINTER(1);

    if (state2 == NULL) {
        PG_RETURN_POINTER(state1);
    }

    if (state1 == NULL) {
        old_context = MemoryContextSwitchTo(agg_context);

        state1 = copy_state(state2);
        state1->aggctx = agg_context;
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

Datum funnel_result_reducer(PG_FUNCTION_ARGS) {

    ArrayType *array;
    Oid arrayElementType;
    int16 arrayElementTypeWidth;
    bool arrayElementTypeByValue;
    char arrayElementTypeAlignmentCode;

    Datum *arrayContent;
    bool *arrayNullFlags;
    int arrayLength;

    CHECK_AGG_CONTEXT("funnel_result_reducer", fcinfo);

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

    //init
    int maxStep = 0,i,j;
    Datum *result = (Datum *) palloc(sizeof(Datum) * FUNNEL_MAX_STEP);
    for (i = 0; i < FUNNEL_MAX_STEP; i++) {
        result[i] = Int32GetDatum(0);
    }

    //update
    for (i = 0; i < arrayLength; i++) {
        int32 step = DatumGetInt32(arrayContent[i]);
        if (step > maxStep) {
            maxStep = step;
        }

        //step start from 1
        for (j = 1; j <= step; j++) {
            result[j - 1] = Int32GetDatum(DatumGetInt32(result[j - 1]) + 1);
        }
    }

    ArrayType *resultArray = construct_array(result, maxStep, arrayElementType,
                                             arrayElementTypeWidth, true,
                                             arrayElementTypeAlignmentCode);

    pfree(result);
    PG_RETURN_ARRAYTYPE_P(resultArray);
}

static int compare(const void *p1, const void *p2) {
    //sort by timestamp asc,step desc
    if(GET_FUNNEL_TIME((*(int64 *) p1)) > GET_FUNNEL_TIME((*(int64 *) p2))) {
        return 1;
    } else if (GET_FUNNEL_TIME((*(int64 *) p1)) < GET_FUNNEL_TIME((*(int64 *) p2))) {
        return -1;
    } else {
        return GET_FUNNEL_STEP((*(int64 *) p2)) - GET_FUNNEL_STEP((*(int64 *) p1));
    }
}

static funnel_state *init(int8 totalStep, int64 windowSize, MemoryContext ctx) {
    funnel_state *p = (funnel_state *) palloc(sizeof(funnel_state));

    p->aggctx = ctx;
    p->totalStep = totalStep;
    p->windowSize = windowSize;
    p->nfirst = 0;

    p->ncapacity = ARRAY_INIT_SIZE;
    p->nlength = 0;
    p->data = (int64 *) palloc(p->ncapacity * ELEMENT_TYPE_LENGTH);

    return p;
}

static int16 detailReduce(funnel_state *state) {

    if (state == NULL || state->nfirst == 0) {
        return 0;
    }

    int16 maxStep = 0;

    //sort
    qsort(state->data, state->nlength, ELEMENT_TYPE_LENGTH, compare);

    funnel_unit *funnelUnitArray = (funnel_unit *) palloc(sizeof(funnel_unit) * ARRAY_INIT_SIZE);
    int len = 0, capacity = ARRAY_INIT_SIZE;

    int i,j;
    for (i = 0; i < state->nlength; i++) {
        long ts = GET_FUNNEL_TIME(state->data[i]);
        int step = GET_FUNNEL_STEP(state->data[i]);

        if (step == 1) {
            if (len + 1 > capacity) {
                capacity = capacity * 2;
                funnelUnitArray = (funnel_unit *) repalloc(funnelUnitArray, sizeof(funnel_unit) * capacity);
            }

            funnelUnitArray[len].currentStep = 1;
            funnelUnitArray[len].startTime = ts;
            funnelUnitArray[len].currentStepTime = ts;
            ++len;

            if (step > maxStep) {
                maxStep = step;
            }
        } else {
            for (j = len - 1; j >= 0; j--) {
                funnel_unit *cur = funnelUnitArray + j;
                if (cur->currentStep + 1 == step && ts > cur->currentStepTime &&
                    ts <= (cur->startTime + state->windowSize)) {
                    cur->currentStep += 1;
                    cur->currentStepTime = ts;

                    if (cur->currentStep > maxStep) {
                        maxStep = cur->currentStep;
                        break;
                    }
                }
            }
        }

        if (maxStep == state->totalStep) {
            break;
        }
    }


    pfree(funnelUnitArray);

#if DEBUG_LOG
    elog(INFO, "detailReduce: totalStep=%d windowSize=%d ncapacity=%d nlength=%d result=%d.",
        state->totalStep, state->windowSize, state->ncapacity, state->nlength, maxStep);
#endif
    return maxStep;
}

static void add(funnel_state *state, int64 value) {
    if (state->nlength > FUNNEL_MAX_DATA_SIZE) {
        elog(WARNING, "funnel data size (%d) is larger than %d.", state->nlength, FUNNEL_MAX_DATA_SIZE);
        return;
    }

    if (GET_FUNNEL_STEP(value) == 1) {
        state->nfirst += 1;
    }

    if (state->nlength + 1 > state->ncapacity) {
        if (state->ncapacity * ELEMENT_TYPE_LENGTH / 0.8 < ALLOCSET_SEPARATE_THRESHOLD) {
            state->ncapacity *= 2;
        } else {
            state->ncapacity /= 0.8;
        }
        state->data = repalloc(state->data, state->ncapacity * ELEMENT_TYPE_LENGTH);
    }

    state->data[state->nlength] = value;
    state->nlength += 1;
}

static funnel_state *copy_state(funnel_state *state)
{
    funnel_state *copy = (funnel_state *) palloc(sizeof(funnel_state));
    copy->totalStep = state->totalStep;
    copy->windowSize = state->windowSize;

    copy->nfirst = state->nfirst;
    copy->nlength = state->nlength;
    copy->ncapacity = state->ncapacity;

    copy->data = palloc(copy->ncapacity * ELEMENT_TYPE_LENGTH);

    memcpy(copy->data, state->data, state->nlength * ELEMENT_TYPE_LENGTH);

    return copy;
}