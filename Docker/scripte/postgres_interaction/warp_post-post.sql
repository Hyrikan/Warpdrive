-- Reads pg1_sf1_lineitem excluding decimals and dates from postgresql and prints it.

-- create the warpFDW as extension
CREATE EXTENSION IF NOT EXISTS warpFDW;

CREATE SERVER IF NOT EXISTS warpFDWTest1
 FOREIGN DATA WRAPPER warpFDW
 OPTIONS (dbname 'db1', host '10.5.1.21', port '13337', fetch_size '250000', use_remote_estimate 'true', receiveinrow 'True');

CREATE USER MAPPING IF NOT EXISTS FOR postgres
  SERVER warpFDWTest1
  OPTIONS (user 'postgres', password 'post1234');

-- IMPORT FOREIGN SCHEMA public LIMIT TO (pg1_sf1_region) FROM SERVER test1 INTO public;
CREATE FOREIGN TABLE IF NOT EXISTS test_lineitem(
    l_orderkey      BIGINT,
    l_partkey       INTEGER,
    l_suppkey       INTEGER,
    l_linenumber    INTEGER,
--    l_quantity      DECIMAL(12, 2),
--    l_extendedprice DECIMAL(12, 2),
--    l_discount      DECIMAL(12, 2),
--    l_tax           DECIMAL(12, 2),
    l_returnflag    CHAR(1),
    l_linestatus    CHAR(1),
--    l_shipdate      DATE,
--    l_commitdate    DATE,
--    l_receiptdate   DATE,
    l_shipinstruct  CHAR(25),
    l_shipmode      CHAR(10),
    l_comment       VARCHAR(44)
)
SERVER warpFDWTest1
OPTIONS (query 'SELECT  l_orderkey, l_partkey, l_suppkey,
l_linenumber, l_returnflag, l_linestatus,
l_shipinstruct, l_shipmode, l_comment FROM pg1_sf1_lineitem LIMIT 20000;');

SELECT * FROM test_lineitem;

DROP FOREIGN TABLE test_lineitem;
