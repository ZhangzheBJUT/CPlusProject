DROP TABLE IF EXISTS event_log;
CREATE TABLE IF NOT EXISTS event_log (
    token VARCHAR(48) NOT NULL,
    distinct_id VARCHAR(48) NOT NULL,
    event_name VARCHAR(32) NOT NULL,
    channel VARCHAR(32) NOT NULL,
    event_time BIGINT NOT NULL,
    ds VARCHAR(32) NOT NULL
);

INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19A','step2','XiaoMi',1583475650000,'20200306');
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19B','step1','XiaoMi',1583475660000,'20200306');
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19B','step2','XiaoMi',1583475660001,'20200306');
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19B','step3','XiaoMi',1583475660002,'20200306');
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19B','step4','XiaoMi',1583475660003,'20200306');
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c18B','step1','XiaoMi',1583475660000,'20200306');

INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19C','step1','HuaWei',1583475660000,'20200306');
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19C','step2','HuaWei',1583475660001,'20200306');
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19C','step3','HuaWei',1583475660002,'20200306');
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19C','step4','HuaWei',1583475660003,'20200306');
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19D','step1','HuaWei',1583475660001,'20200306');
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19E','step1','HuaWei',1583475660002,'20200306');
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19E','step2','HuaWei',1583475660003,'20200306');


-- \echo test query 1
SELECT distinct_id,retention_count(ds,'YYYYmmdd',type,'day') AS retention_state
  FROM (
    SELECT 0 AS type,distinct_id,ds
    FROM event_log
    WHERE token = '5aa67c90f29d980c06000125' AND event_name='step1'
    GROUP BY distinct_id,ds
    UNION ALL
    SELECT 1 AS type,distinct_id,ds
    FROM event_log
    WHERE token = '5aa67c90f29d980c06000125' AND event_name='step2'
    GROUP BY distinct_id
  )t1
GROUP BY distinct_id;

SELECT retention_sum(retention_state) AS result
FROM (
  SELECT distinct_id,retention_count(ds,'YYYYmmdd',type,'day') AS retention_state
  FROM (
    SELECT 0 AS type,distinct_id,ds
    FROM event_log
    WHERE token = '5aa67c90f29d980c06000125' AND event_name='step1'
    GROUP BY distinct_id,ds
    UNION ALL
    SELECT 1 AS type,distinct_id,ds
    FROM event_log
    WHERE token = '5aa67c90f29d980c06000125' AND event_name='step2'
    GROUP BY distinct_id,ds
  )t1
  GROUP BY distinct_id
)t2;

\echo test query 2
SELECT channel,retention_sum(retention_state) AS result
FROM (
  SELECT distinct_id,channel,retention_count(ds,'YYYYmmdd',type,'day') AS retention_state
  FROM (
    SELECT 0 AS type,distinct_id,ds,channel
    FROM event_log
    WHERE token = '5aa67c90f29d980c06000125' AND event_name='step1'
    GROUP BY distinct_id,ds,channel
    UNION ALL
    SELECT 1 AS type,distinct_id,ds,channel
    FROM event_log
    WHERE token = '5aa67c90f29d980c06000125' AND event_name='step2'
    GROUP BY distinct_id,ds,channel
  )t1
  GROUP BY distinct_id,channel
)t2
GROUP BY channel;