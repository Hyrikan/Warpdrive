SELECT l_orderkey, sum(l_extendedprice * (1-l_discount)) as revenue, o_orderdate, o_shippriority FROM (SELECT * FROM 
                        (SELECT l_orderkey, l_extendedprice, l_discount FROM pg1_sf1_lineitem WHERE l_shipdate > date '1995-03-15') AS lineitem_localview, 
                        (SELECT o_orderkey, o_orderdate, o_shippriority FROM 
                        (SELECT o_orderkey, o_custkey, o_orderdate, o_shippriority FROM pg1_sf1_orders WHERE o_orderdate < date '1995-03-15') AS orders_localview, 
                        (SELECT c_custkey FROM pg1_sf1_customer WHERE c_mktsegment = 'BUILDING') AS customer_localview WHERE c_custkey = o_custkey) 
                        AS ordercustomer_localview WHERE l_orderkey = o_orderkey) AS ordercustomerlineitem_joinview
                        GROUP BY l_orderkey, o_orderdate, o_shippriority 
                        ORDER BY revenue desc, o_orderdate 
                        LIMIT 20;
