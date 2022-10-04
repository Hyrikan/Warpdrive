-- this script queries a table of jmdb2 maria server from another maria server using the connect engine.

INSTALL SONAME 'ha_connect';

DROP TABLE IF EXISTS mdbF_sf1_region;
CREATE TABLE mdbF_sf1_region
ENGINE = CONNECT
TABLE_TYPE = JDBC
Block_size = 250000
DBNAME = mdb
TABNAME = mdb_sf1_region
CONNECTION = 'jdbc:mysql://jmdb-2:3306/mdb?user=mariadb&password=maria1234'
OPTION_LIST = 'WRAPPER=MariadbInterface';

SELECT * FROM mdbF_sf1_region;
