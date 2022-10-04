--Imports the region table from mdb1 mariadb server to the current postgres server and prints it.
CREATE EXTENSION IF NOT EXISTS odbc_fdw;

DROP SERVER IF EXISTS mdb1 CASCADE;
CREATE SERVER mdb1
 FOREIGN DATA WRAPPER odbc_fdw
 OPTIONS (odbc_driver 'MariaDB', odbc_server '10.5.1.11', odbc_port '3306');

CREATE USER MAPPING FOR postgres
  SERVER mdb1
  OPTIONS (odbc_UID 'mariadb', odbc_PWD 'maria1234');

DROP FOREIGN TABLE IF EXISTS mdb_sf1_region CASCADE;

CREATE FOREIGN TABLE mdb_sf1_region(
	r_regionkey INTEGER,
	r_name CHAR(25),
	r_comment VARCHAR(125)) 
	SERVER mdb1 OPTIONS ( 
	odbc_DATABASE 'mdb', 
	table 'mdb_sf1_region',
	sql_query 'select * FROM mdb_sf1_region', 
	sql_count 'select 1 ' ); 




SELECT * FROM mdb_sf1_region;
