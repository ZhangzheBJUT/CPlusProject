#include <AggregateFunctions/AggregateFunctionFactory.h>

#include <AggregateFunctions/AggregateFunctionRetentionCount.h>
#include <AggregateFunctions/FactoryHelpers.h>

namespace DB
{
    AggregateFunctionPtr createAggregateFunctionRetentionCount(const std::string & name, const DataTypes & arguments, const Array & params)
    {
        if (arguments.size() != 2)
            throw Exception{"Aggregate function " + name + " requires two arguments.",
                            ErrorCodes::NUMBER_OF_ARGUMENTS_DOESNT_MATCH};

        if (params.size() != 1)
            throw Exception{"Aggregate function " + name + " requires exactly one parameter.",
                            ErrorCodes::NUMBER_OF_ARGUMENTS_DOESNT_MATCH};


        return std::make_shared<AggregateFunctionRetentionCount>(arguments, params);
    }

    void registerAggregateFunctionRetentionCount(AggregateFunctionFactory & factory)
    {
        factory.registerFunction("retention_count", createAggregateFunctionRetentionCount, AggregateFunctionFactory::CaseInsensitive);
    }
}