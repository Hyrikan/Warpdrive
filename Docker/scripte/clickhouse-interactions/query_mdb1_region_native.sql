-- this sql script drops the table mdb1_sf1_region from jmdb-1 docker container, recreates it and queries it.

DROP TABLE IF EXISTS mdb1_sf1_region;
CREATE TABLE mdb1_sf1_region (
  r_regionkey UInt64,
  r_name String,
  r_comment String
)
ENGINE = MySQL('10.5.1.11:3306','mdb','mdb_sf1_region','mariadb','maria1234');
SELECT * FROM mdb1_sf1_region;
