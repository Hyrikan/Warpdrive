-- this script queries a table of jpg-1 postgres server from another maria server using the connect engine.

INSTALL SONAME 'ha_connect';

DROP TABLE IF EXISTS pg1_sf1_region;
CREATE TABLE pg1_sf1_region
ENGINE = CONNECT
TABLE_TYPE = JDBC
Block_size = 250000
DBNAME = db1
TABNAME = pg1_sf1_region
CONNECTION = 'jdbc:postgresql://jpg-1:5432/db1?user=postgres&password=post1234'
OPTION_LIST = 'Wrapper=PostgresqlInterface';

SELECT * FROM pg1_sf1_region;
