# This script imports tpc-h test data from the previous generated test data volume into
# the database of a clickhouse docker container.
#
# CAREFUL! Only use this script once! One can insert the same data twice in Clickhouse!
#!/bin/bash
set -x
docker exec -u root jclick-$1 bash -c "chmod +x import_tpch_sf10.sh"
docker exec jclick-$1 bash -c "bash import_tpch_sf10.sh"
