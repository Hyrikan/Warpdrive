
-- this script querys a table from jpg-1 via jdbc

SELECT * FROM jdbc('jpg-1', 'select * from pg1_sf1_region');
