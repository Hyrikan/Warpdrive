CREATE EXTENSION IF NOT EXISTS jdbc_fdw;

DROP SERVER IF EXISTS jdbcpos2 CASCADE;
CREATE SERVER jdbcpos2 FOREIGN DATA WRAPPER jdbc_fdw OPTIONS(
drivername 'org.postgresql.Driver',
url 'jdbc:postgresql://10.5.1.22:5432/db1',
querytimeout '10',
jarfile '/JDBC_FDW/jdbcdriver/postgresql-42.3.3.jar',
maxheapsize '1000'
);

CREATE USER MAPPING FOR postgres SERVER jdbcpos2 OPTIONS(
username 'postgres',
password 'post1234'
);

CREATE FOREIGN TABLE jdbcpos2_sf1_region(
r_regionkey INTEGER,
r_name CHAR(25),
r_comment VARCHAR(125)
)
SERVER jdbcpos2
OPTIONS(
query 'SELECT * FROM pg2_sf1_region;');

SELECT * FROM jdbcpos2_sf1_region;
