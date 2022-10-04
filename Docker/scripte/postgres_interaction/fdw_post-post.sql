--Reads the region table from pg-1 to the current postgres server and prints it.

CREATE EXTENSION IF NOT EXISTS postgres_fdw;

DROP SERVER IF EXISTS pos1 CASCADE;
CREATE SERVER pos1
 FOREIGN DATA WRAPPER postgres_fdw
 OPTIONS (dbname 'db1', host 'jpg-1', port '5432', fetch_size '250000', use_remote_estimate 'true');

CREATE USER MAPPING FOR postgres
  SERVER pos1
  OPTIONS (user 'postgres', password 'post1234');

DROP FOREIGN TABLE IF EXISTS pg1_sf1_region CASCADE;

IMPORT FOREIGN SCHEMA public LIMIT TO (pg1_sf1_region) FROM SERVER pos1 INTO public;



SELECT * FROM pg1_sf1_region;
