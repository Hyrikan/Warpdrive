-- this script querys a table from jmdb-1 via jdbc

SELECT * FROM jdbc('jmdb-1', 'select * from mdb_sf1_region');
