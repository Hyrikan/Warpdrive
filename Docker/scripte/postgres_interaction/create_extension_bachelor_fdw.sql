-- creates the bachelor_fdw as extension

CREATE EXTENSION IF NOT EXISTS bachelor_fdw;

CREATE SERVER IF NOT EXISTS test1
 FOREIGN DATA WRAPPER bachelor_fdw
 OPTIONS (dbname 'db1', host 'jpg-1', port '5432', fetch_size '250000', use_remote_estimate 'true');

CREATE USER MAPPING IF NOT EXISTS FOR postgres
  SERVER test1
  OPTIONS (user 'postgres', password '123456');

-- IMPORT FOREIGN SCHEMA public LIMIT TO (pg1_sf1_region) FROM SERVER test1 INTO public;
CREATE FOREIGN TABLE IF NOT EXISTS anynametesttable(
	ID	integer NOT NULL,
	vorname	varchar(20) NOT NULL,
	nachname varchar(20) NOT NULL)
SERVER test1;


\echo "SELECT * FROM anynametesttable WHERE vorname='Lara';"
SELECT * FROM anynametesttable WHERE vorname='Lara';

