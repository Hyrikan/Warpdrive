#!/bin/bash/

# this script creates tables for tpch and subsequently inserts test data into these tables.


clickhouse-client --multiquery < create_tpch_sf10.sql

clickhouse-client --format_csv_delimiter="|" --query="INSERT INTO click_sf10_region FORMAT CSV" < /data/sf10/region.tbl
clickhouse-client --format_csv_delimiter="|" --query="INSERT INTO click_sf10_nation FORMAT CSV" < /data/sf10/nation.tbl
clickhouse-client --format_csv_delimiter="|" --query="INSERT INTO click_sf10_supplier FORMAT CSV" < /data/sf10/supplier.tbl
clickhouse-client --format_csv_delimiter="|" --query="INSERT INTO click_sf10_customer FORMAT CSV" < /data/sf10/customer.tbl
clickhouse-client --format_csv_delimiter="|" --query="INSERT INTO click_sf10_orders FORMAT CSV" < /data/sf10/orders.tbl
clickhouse-client --format_csv_delimiter="|" --query="INSERT INTO click_sf10_part FORMAT CSV" < /data/sf10/part.tbl
clickhouse-client --format_csv_delimiter="|" --query="INSERT INTO click_sf10_partsupp FORMAT CSV" < /data/sf10/partsupp.tbl
clickhouse-client --format_csv_delimiter="|" --query="INSERT INTO click_sf10_lineitem FORMAT CSV" < /data/sf10/lineitem.tbl
