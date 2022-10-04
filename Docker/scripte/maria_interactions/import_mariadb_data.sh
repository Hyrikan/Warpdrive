# This script imports tpc-h test data from the previous generated data volume into the first
# mariadb docker container
#!/bin/bash
set -x
docker exec jmdb-$1 bash -c "mysql mdb < create_tpch_sf1.sql -u mariadb -pmaria1234"
docker exec jmdb-$1 bash -c "mysql mdb < import_tpch_sf1.sql -u mariadb -pmaria1234"

#docker exec mdb1 bash -c "mysql mdb < create_tpch_sf10.sql -u mariadb -p123456"
##docker exec mdb1 bash -c "mysql mdb < import_tpch_sf10.sql -u mariadb -p123456"
