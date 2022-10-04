CREATE TABLE IF NOT EXISTS click_sf1_supplier
(
    s_suppkey   Int32,
    s_name      String,
    s_address   String,
    s_nationkey Int32,
    s_phone     String,
    s_acctbal   Decimal32(2),
    s_comment   String
)
ENGINE = MergeTree
ORDER BY s_suppkey;

CREATE TABLE IF NOT EXISTS click_sf1_part
(
    p_partkey     Int32,
    p_name        String,
    p_mfgr        String,
    p_brand       String,
    p_type        String,
    p_size        Int32,
    p_container   String,
    p_retailprice Decimal32(2),
    p_comment     String
)
ENGINE = MergeTree
ORDER BY p_partkey;

CREATE TABLE IF NOT EXISTS click_sf1_partsupp
(
    ps_partkey    Int32,
    ps_suppkey    Int32,
    ps_availqty   Int32,
    ps_supplycost Decimal32(2),
    ps_comment    String
)
ENGINE = MergeTree
ORDER BY (ps_partkey,ps_suppkey);

CREATE TABLE IF NOT EXISTS click_sf1_customer
(
    c_custkey    Int32,
    c_name       String,
    c_address    String,
    c_nationkey  Int32,
    c_phone      String,
    c_acctbal    Decimal(20,2),
    c_mktsegment String,
    c_comment    String
)
ENGINE = MergeTree
ORDER BY c_custkey;

CREATE TABLE IF NOT EXISTS click_sf1_orders
(
    o_orderkey      Int64,
    o_custkey       Int32,
    o_orderstatus   String,
    o_totalprice    Decimal32(2),
    o_orderdate     Date,
    o_orderpriority String,
    o_clerk         String,
    o_shippriority  Int32,
    o_comment       String
)
ENGINE = MergeTree
ORDER BY o_orderkey;

CREATE TABLE IF NOT EXISTS click_sf1_lineitem
(
    l_orderkey      Int64,
    l_partkey       Int32,
    l_suppkey       Int32,
    l_linenumber    Int32,
    l_quantity      Decimal(12,2),
    l_extendedprice Decimal(12,2),
    l_discount      Decimal(12,2),
    l_tax           Decimal(12,2),
    l_returnflag    String,
    l_linestatus    String,
    l_shipdate      Date,
    l_commitdate    Date,
    l_receiptdate   Date,
    l_shipinstruct  String,
    l_shipmode      String,
    l_comment       String
)
ENGINE = MergeTree
ORDER BY (l_orderkey, l_partkey, l_suppkey);

CREATE TABLE IF NOT EXISTS click_sf1_nation
(
    n_nationkey Int32,
    n_name      String,
    n_regionkey Int32,
    n_comment   String
)
ENGINE = MergeTree
ORDER BY n_nationkey;

CREATE TABLE IF NOT EXISTS click_sf1_region
(
    r_regionkey Int32,
    r_name      String,
    r_comment   String
)
ENGINE = MergeTree
ORDER BY r_regionkey;
