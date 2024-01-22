/*********************************************************
 *   Author : zhangzhe
 * Description: Calculate retention -- retention_count(granularity)(behaviorType, date)
*********************************************************/

#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include <Columns/ColumnsNumber.h>
#include <Columns/ColumnArray.h>
#include <DataTypes/DataTypeArray.h>
#include <DataTypes/DataTypeString.h>
#include <DataTypes/DataTypesNumber.h>
#include <DataTypes/DataTypeDate.h>

#include <IO/ReadHelpers.h>
#include <IO/WriteHelpers.h>

#include <common/logger_useful.h>
#include <common/LocalDateTime.h>
#include <common/DateLUTImpl.h>

#include <AggregateFunctions/IAggregateFunction.h>

namespace DB
{

struct ComparePairTwo final
{
    template <typename T1, typename T2>
    bool operator()(const std::pair<T1, T2> & lhs, const std::pair<T1, T2> & rhs) const
    {
        if (lhs.second < rhs.second )
            return true;
        else if (lhs.second == rhs.second)
            return lhs.first < rhs.first;
        else
            return false;
    }
};


struct AggregateFunctionRetentionCountData
{
    /**
     * BehaviorType: 0-initial 1-return
     */
    using BehaviorTypeDayNum = std::pair<UInt8, UInt16>;
    static constexpr size_t bytes_on_stack = 64;
    using BehaviorTypeDayNumList = PODArray<BehaviorTypeDayNum, bytes_on_stack, AllocatorWithStackMemory<Allocator<false>, bytes_on_stack>>;
    BehaviorTypeDayNumList behaviorTypeDayNumList;

    /**
     *  Compare Funciton
     */
    using Comparator = ComparePairTwo;

    size_t size() const
    {
        return behaviorTypeDayNumList.size();
    }

    void add(UInt8 aBehaviorType, UInt16 aDayNum)
    {
        behaviorTypeDayNumList.emplace_back(aBehaviorType, aDayNum);
    }

    void merge(const AggregateFunctionRetentionCountData & rhs)
    {
        behaviorTypeDayNumList.insert(std::begin(rhs.behaviorTypeDayNumList), std::end(rhs.behaviorTypeDayNumList));
    }

    void sort()
    {
        std::stable_sort(std::begin(behaviorTypeDayNumList), std::end(behaviorTypeDayNumList), Comparator{});
    }

    void write(WriteBuffer & buf) const
    {
        writeBinary(behaviorTypeDayNumList.size(), buf);

        for (const auto & data : behaviorTypeDayNumList)
        {
            writeBinary(data.first, buf);
            writeBinary(data.second, buf);
        }
    }

    void read(ReadBuffer & buf)
    {
        size_t size;
        readBinary(size, buf);

        behaviorTypeDayNumList.clear();
        behaviorTypeDayNumList.reserve(size);

        UInt8 behaviorType;
        UInt16 dayNum;

        for (size_t i = 0; i < size; ++i)
        {
            readBinary(behaviorType, buf);
            readBinary(dayNum, buf);
            behaviorTypeDayNumList.emplace_back(behaviorType, dayNum);
        }
    }
};

/** Calculate retention
 *
 *  function description:  retention_count(granularity)(type, date)
 *     granularity : day,week,month
 *            type : 0 - initial, 1 - return
 *
 *  function Usage:
 *      retention_count(0, toDate('2020-12-12'))
 */
class AggregateFunctionRetentionCount final :
        public IAggregateFunctionDataHelper<AggregateFunctionRetentionCountData, AggregateFunctionRetentionCount>
{
public:

    static constexpr UInt8  BEHAVIOR_INITIAL = 0;
    static constexpr UInt8  BEHAVIOR_RETURN  = 1;

    static constexpr UInt16 MAX_INTERVAL_VALUE = 365;

    static constexpr UInt8  RETENTION_TYPE_INITIAL_ONLY = 0;
    static constexpr UInt8  RETENTION_TYPE_BOTH         = 1;
    static constexpr UInt8  RETENTION_TYPE_RETURN_ONLY  = 2;
    static constexpr UInt8  RETENTION_TYPE_NONE         = 99;


    AggregateFunctionRetentionCount(const DataTypes & arguments, const Array & params)
            : IAggregateFunctionDataHelper<AggregateFunctionRetentionCountData, AggregateFunctionRetentionCount>(arguments, params)
    {
        // check argument type and value
        const auto behavior_arg = arguments[0].get();
        if (!WhichDataType(behavior_arg).isUInt8())
            throw Exception{
                    "Illegal type " + behavior_arg->getName() + " of first argument of aggregate function " + getName()
                    + ", must be UInt8(0 or 1).", ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT};

        const auto date_arg = arguments[1].get();
        if (!WhichDataType(date_arg).isDate())
            throw Exception{
                    "Illegal type " + date_arg->getName() + " of second argument of aggregate function " + getName()
                    + ", must be Date.", ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT};

        // check param value
        std::string param_arg = params[0].safeGet<String>();
        if (param_arg != "day" && param_arg != "week" && param_arg != "month")
            throw Exception{
                    "Illegal value the first param of aggregate function " + getName()
                    + ", must be day(week or month).", ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT};
        this->granularity = param_arg;
    }

    String getName() const override
    {
        return "retention_count";
    }

    DataTypePtr getReturnType() const override
    {
        return std::make_shared<DataTypeArray>(std::make_shared<DataTypeString>());
    }

    void add(AggregateDataPtr place, const IColumn ** columns, size_t row_num, Arena *) const override
    {
        const auto behaviorType = static_cast<const ColumnVector<UInt8> *>(columns[0])->getData()[row_num];
        const auto dayNum = static_cast<const ColumnVector<UInt16> *>(columns[1])->getData()[row_num];

        if (behaviorType != BEHAVIOR_INITIAL && behaviorType != BEHAVIOR_RETURN)
            throw Exception{ "Illegal value first argument of aggregate function " + getName() +
                             ", must be UInt8(0 or 1).", ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT};

        this->data(place).add(behaviorType, dayNum);
    }

    void merge(AggregateDataPtr place, ConstAggregateDataPtr rhs, Arena *) const override
    {
        this->data(place).merge(this->data(rhs));
    }

    void serialize(ConstAggregateDataPtr place, WriteBuffer & buf) const override
    {
        this->data(place).write(buf);
    }

    void deserialize(AggregateDataPtr place, ReadBuffer & buf, Arena *) const override
    {
        this->data(place).read(buf);
    }

    void insertResultInto(ConstAggregateDataPtr place, IColumn & to) const override
    {
        //get retention result data
        const AggregateFunctionRetentionCountData & data = this->data(place);
        //TODO:右值优化
        const std::vector<std::string> result_vec = getRetentionResult(data);

        ColumnArray & arr_to = static_cast<ColumnArray &>(to);
        //update result size
        ColumnArray::Offsets & offsets_to = arr_to.getOffsets();
        offsets_to.push_back(offsets_to.back() + result_vec.size());

        //update result data
        IColumn & data_to = arr_to.getData();
        for (auto & elem : result_vec)
        {
            data_to.insertData(elem.data(), elem.size());
        }
    }

    const char * getHeaderFilePath() const override { return __FILE__; }

private:
    UInt16 addInterval(UInt16 &lastDay, UInt16 interval) const
    {
        time_t result;

        if (this->granularity == "day")
        {
            result = DateLUT::instance().addDays(DateLUT::instance().toDate(DayNum(lastDay)), interval);
        }
        else if (this->granularity == "week")
        {
            result = DateLUT::instance().addWeeks(DateLUT::instance().toDate(DayNum(lastDay)), interval);
        }
        else
        {
            result = DateLUT::instance().addMonths(DateLUT::instance().toDate(DayNum(lastDay)), interval);
        }

        result = DateLUT::instance().toDayNum(result);

        return result;
    }

    void updateRetentionType(std::vector<std::pair<UInt16, UInt8>> &retentionType, UInt16 &lastDay, UInt16 dayNum, UInt8 type) const
    {
        if (lastDay != 0)
        {
            int i = 0;
            while (i++ < MAX_INTERVAL_VALUE)
            {
                UInt16 next = addInterval(lastDay, 1);
                if (next != dayNum)
                {
                    retentionType.push_back(std::make_pair(next, RETENTION_TYPE_NONE));
                    lastDay = next;
                }
                else
                {
                    break;
                }
            }

            if (i == MAX_INTERVAL_VALUE)
            {
                LOG_ERROR(log, "retention max interval is greater than " << MAX_INTERVAL_VALUE);
            }
        }

        retentionType.push_back(std::make_pair(dayNum, type));
        lastDay = dayNum;
    }

    std::vector<std::string> getRetentionResult(const AggregateFunctionRetentionCountData & data) const
    {
        std::vector<std::string> resultList;

        if (data.size() == 0)
        {
            return resultList;
        }

        const_cast<AggregateFunctionRetentionCountData &>(data).sort();

        //1.get retention type
        std::vector<std::pair<UInt16, UInt8>> retentionTypeList;
        UInt8 preType, type;
        UInt16 preday, day, total = data.size(), lastDay = 0 ,i = 0, j = 1;

        while ( j< total && i < total)
        {
            preType = data.behaviorTypeDayNumList[i].first;
            preday = data.behaviorTypeDayNumList[i].second;

            type = data.behaviorTypeDayNumList[j].first;
            day = data.behaviorTypeDayNumList[j].second;

            if (day == preday)
            {
                if (preType != type)
                {
                    updateRetentionType(retentionTypeList, lastDay, preday, RETENTION_TYPE_BOTH);
                }
                else
                {
                    LOG_ERROR(log, "behaviorType:" << day << ",preType="
                           << toString(preType) << ",type=" << toString(type) << "," << this->granularity );
                }
                i += 2;
            }
            else
            {
                if (preType == BEHAVIOR_INITIAL)
                {
                    updateRetentionType(retentionTypeList, lastDay, preday, RETENTION_TYPE_INITIAL_ONLY);
                }
                else
                {
                    updateRetentionType(retentionTypeList, lastDay, preday, RETENTION_TYPE_RETURN_ONLY);
                }
                i += 1;
            }

            j = i + 1;
        }

        if (i < total)
        {
            preType = data.behaviorTypeDayNumList[i].first;
            preday = data.behaviorTypeDayNumList[i].second;

            if (preType == BEHAVIOR_INITIAL)
            {
                updateRetentionType(retentionTypeList, lastDay, preday, RETENTION_TYPE_INITIAL_ONLY);
            }
            else
            {
                updateRetentionType(retentionTypeList, lastDay, preday, RETENTION_TYPE_RETURN_ONLY);
            }
        }

        //2.get retention result
        int length = retentionTypeList.size();
        for (int outerIndex = 0; outerIndex < length; ++outerIndex)
        {
            UInt8 retentionType = retentionTypeList[outerIndex].second;
            if (retentionType == RETENTION_TYPE_INITIAL_ONLY || retentionType == RETENTION_TYPE_BOTH)
            {
                UInt16 dayNum = retentionTypeList[outerIndex].first;
                UInt16 innerIndex = 1, iterator = outerIndex + 1, size = 1, offset = 1;

                //result cache
                std::stringstream retentionDataBuilder;
                retentionDataBuilder << DateLUT::instance().toNumYYYYMMDD(DateLUT::instance().toDate(DayNum(dayNum)))
                                     << ":" << "1";

                //day retention
                if (this->granularity == "day")
                {
                    while ((iterator < length) && (innerIndex <= 7 || innerIndex == 14 || innerIndex == 30))
                    {
                        retentionType = retentionTypeList[iterator].second;

                        if (retentionType >= RETENTION_TYPE_BOTH && retentionType != RETENTION_TYPE_NONE)
                        {
                            while (offset > size)
                            {
                                retentionDataBuilder << 0;
                                size += 1;
                            }

                            retentionDataBuilder << "1";
                            size += 1;
                        }
                        offset += 1;


                        if (innerIndex < 7)
                        {
                            innerIndex = innerIndex + 1;
                        }
                        else if (innerIndex == 7)
                        {
                            innerIndex = 14;
                        }
                        else if (innerIndex == 14)
                        {
                            innerIndex = 30;
                        }
                        else
                        {
                            break;
                        }

                        iterator = outerIndex + innerIndex;
                    }
                }
                //week or month retention
                else if (this->granularity == "week" || this->granularity == "month")
                {
                    while (iterator < length && innerIndex <= 9)
                    {
                        retentionType = retentionTypeList[iterator].second;

                        if (retentionType >= RETENTION_TYPE_BOTH && retentionType != RETENTION_TYPE_NONE)
                        {
                            while (offset > size)
                            {
                                retentionDataBuilder << 0;
                                size += 1;
                            }

                            retentionDataBuilder << "1";
                            size += 1;
                        }
                        offset += 1;

                        innerIndex = innerIndex + 1;
                        iterator = outerIndex + innerIndex;
                    }
                }

                resultList.push_back(retentionDataBuilder.str());
            }//if
        }

        return resultList;
    }

    std::string granularity;
    Logger * log = &Logger::get("AggregateFunctionRetentionCount");
};

}