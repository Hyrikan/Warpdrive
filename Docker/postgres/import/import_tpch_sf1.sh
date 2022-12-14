#!/bin/bash

set -x

sed "s\CREATE TABLE pg_\CREATE TABLE pg$1_\g" create_tpch_sf1.sql >/tmp/create1.sql
psql -f /tmp/create1.sql db1

psql -d db1 -c "\copy pg$1_sf1_lineitem FROM /data/sf1/lineitem.tbl with CSV DELIMITER '|' QUOTE '\"' ESCAPE '\';"
psql -d db1 -c "\copy pg$1_sf1_supplier FROM /data/sf1/supplier.tbl with CSV DELIMITER '|' QUOTE '\"' ESCAPE '\';"
psql -d db1 -c "\copy pg$1_sf1_region FROM /data/sf1/region.tbl with CSV DELIMITER '|' QUOTE '\"' ESCAPE '\';"
psql -d db1 -c "\copy pg$1_sf1_nation FROM /data/sf1/nation.tbl with CSV DELIMITER '|' QUOTE '\"' ESCAPE '\';"
psql -d db1 -c "\copy pg$1_sf1_orders FROM /data/sf1/orders.tbl with CSV DELIMITER '|' QUOTE '\"' ESCAPE '\';"
psql -d db1 -c "\copy pg$1_sf1_customer FROM /data/sf1/customer.tbl with CSV DELIMITER '|' QUOTE '\"' ESCAPE '\';"
psql -d db1 -c "\copy pg$1_sf1_partsupp FROM /data/sf1/partsupp.tbl with CSV DELIMITER '|' QUOTE '\"' ESCAPE '\';"
psql -d db1 -c "\copy pg$1_sf1_part FROM /data/sf1/part.tbl with CSV DELIMITER '|' QUOTE '\"' ESCAPE '\';"
