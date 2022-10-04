CREATE EXTENSION IF NOT EXISTS jdbc_fdw;

DROP SERVER IF EXISTS jdbcjclick1 CASCADE;
CREATE SERVER jdbcjclick1 FOREIGN DATA WRAPPER jdbc_fdw OPTIONS(
drivername 'com.clickhouse.jdbc.ClickHouseDriver',
url 'jdbc:ch://jclick-1/default?user=default',
querytimeout '10',
jarfile '/JDBC_FDW/jdbcdriver/clickhouse-jdbc-0.3.2-patch11-all.jar',
maxheapsize '1000'
);

CREATE USER MAPPING FOR postgres SERVER jdbcjclick1 OPTIONS(
username 'default',
password '');

CREATE FOREIGN TABLE jdbcclick_sf1_region(
r_regionkey INTEGER,
r_name CHAR(25),
r_comment VARCHAR(125)
)
SERVER jdbcjclick1
OPTIONS(
query 'SELECT * FROM click_sf1_region;');

SELECT * FROM jdbcclick_sf1_region;
