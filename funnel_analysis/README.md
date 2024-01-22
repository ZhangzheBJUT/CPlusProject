funnel_count | funnel_sum aggregate
=============================
## Benchmark
**单机测试环境：8C 16G**

| 数据量     | 漏斗函数运行耗时 | 普通JOIN查询耗时 |
| ---------- | ---------------- | ---------------  | 
| 10,000     | 	76.671 ms       |  130.521 ms      | 
| 100,000    | 1,620.165ms      |  4,512.244 ms     |  
| 1,000,000  | 6,583.714 ms     |  79,393.100ms     |  
| 10,000,000 | 83,258.936ms     | 4min (869,332.236 ms)|  
见：[benchmark.sql](./benchmark_sql/benchmark.sql "benchmark.sql") 

## INSTALL
1. make && make install
2. load test data (benchmark_sql/tmp.sql)
3. create extension funnel_analysis
4. query sql

```
postgres=# create extension funnel_analysis;
CREATE EXTENSION
postgres=#
SELECT channel,funnel_sum(funnel_state)
FROM (
  SELECT distinct_id,channel,funnel_count(3,86400000,event_time,step) AS funnel_state
    FROM (
      SELECT distinct_id,channel,event_time,
             CASE WHEN event_name='step1' THEN 1
                  WHEN event_name='step2' THEN 2
                  WHEN event_name='step3' THEN 3
             END AS step
      FROM event_log_10000
      WHERE token = '5aa67c90f29d980c06000125' AND event_name IN ('step1','step2','step3')
    )t1
  GROUP BY distinct_id,channel
)t2
GROUP BY channel;

   channel   |  funnel_sum
-------------+---------------
 APP Store   | {691,310,94}
 Huawei      | {690,318,117}
 AndroidPlay | {698,313,105}
(3 rows)

Time: 88.963 ms
```
