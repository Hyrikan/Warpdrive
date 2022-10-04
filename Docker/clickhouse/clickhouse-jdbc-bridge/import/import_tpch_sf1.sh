#!/bin/bash/

# this script creates tables for tpch and subsequently inserts test data into these tables.


clickhouse-client --multiquery < create_tpch_sf1.sql

clickhouse-client --format_csv_delimiter="|" --query="INSERT INTO click_sf1_region FORMAT CSV" < /data/sf1/region.tbl
clickhouse-client --format_csv_delimiter="|" --query="INSERT INTO click_sf1_nation FORMAT CSV" < /data/sf1/nation.tbl
clickhouse-client --format_csv_delimiter="|" --query="INSERT INTO click_sf1_supplier FORMAT CSV" < /data/sf1/supplier.tbl
clickhouse-client --format_csv_delimiter="|" --query="INSERT INTO click_sf1_customer FORMAT CSV" < /data/sf1/customer.tbl
clickhouse-client --format_csv_delimiter="|" --query="INSERT INTO click_sf1_orders FORMAT CSV" < /data/sf1/orders.tbl
clickhouse-client --format_csv_delimiter="|" --query="INSERT INTO click_sf1_part FORMAT CSV" < /data/sf1/part.tbl
clickhouse-client --format_csv_delimiter="|" --query="INSERT INTO click_sf1_partsupp FORMAT CSV" < /data/sf1/partsupp.tbl
clickhouse-client --format_csv_delimiter="|" --query="INSERT INTO click_sf1_lineitem FORMAT CSV" < /data/sf1/lineitem.tbl
