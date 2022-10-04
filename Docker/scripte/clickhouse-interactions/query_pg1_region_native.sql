-- this script queries the region table from pg1

DROP TABLE IF EXISTS pg1_sf1_region;
CREATE TABLE pg1_sf1_region (
    r_regionkey UInt64,
    r_name String,
    r_comment String
)
ENGINE = PostgreSQL('10.5.1.21:5432', 'db1', 'pg1_sf1_region', 'postgres', 'post1234');
SELECT * FROM pg1_sf1_region;
