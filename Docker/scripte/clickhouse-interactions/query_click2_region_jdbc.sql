-- this script querys a table from click2 via jdbc

SELECT * FROM jdbc('click2', 'select * from click_sf1_region');
