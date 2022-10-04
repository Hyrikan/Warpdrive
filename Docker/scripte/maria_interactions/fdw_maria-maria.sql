-- this script queries a table of jmdb2 maria server from another maria server using the connect engine.

INSTALL SONAME 'ha_connect';

DROP TABLE IF EXISTS mdbF_sf1_region;
CREATE TABLE mdbF_sf1_region
ENGINE = CONNECT
TABLE_TYPE = MYSQL
DBNAME = mdb
TABNAME = mdb_sf1_region
CONNECTION = 'mysql://mariadb:maria1234@jmdb-2:3306/';

SELECT * FROM mdbF_sf1_region;
