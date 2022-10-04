select * from pg1_sf1_region;
SELECT
    oid, typname
FROM
    pg_type
ORDER BY 
    typname DESC;

SELECT
    column_name, data_type
FROM
    information_schema.columns
WHERE
    table_name = 'pg1_sf1_region';

SELECT typlen, typbyval, typalign, typtype,
       typrelid, typelem, nspname, typname
FROM pg_catalog.pg_type t,
       pg_catalog.pg_namespace n
WHERE t.typnamespace = n.oid
AND t.oid = 1700 OR t.oid = 23;
SELECT (atttypmod - 4) >> 16 & 65535 AS precision,
       (atttypmod - 4) & 65535 AS scale
FROM pg_attribute
WHERE attrelid = 'pg1_sf1_customer'::regclass
  AND attname = 'c_acctbal';
