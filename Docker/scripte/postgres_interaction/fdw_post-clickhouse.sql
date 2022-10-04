-- reads the region table from clickhouse container jclick-1 and prints it

CREATE EXTENSION IF NOT EXISTS clickhouse_fdw;

DROP SERVER IF EXISTS jclick1 CASCADE;
CREATE SERVER jclick1 FOREIGN DATA WRAPPER clickhouse_fdw
OPTIONS(host '10.5.1.31');

CREATE USER MAPPING FOR postgres SERVER jclick1 OPTIONS (user 'default', password '');
IMPORT FOREIGN SCHEMA "default" FROM SERVER jclick1 INTO public;

SELECT * FROM click_sf1_region;

