SELECT * FROM pg1_sf1_lineitem LIMIT 3;

SELECT
    column_name, data_type
FROM
    information_schema.columns
WHERE
    table_name = 'pg1_sf1_lineitem';
