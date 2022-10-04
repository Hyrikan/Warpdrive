# This script imports tpc-h test data from the previous generated test data volume into
# the database of a postgres docker container.
# First parameter is the number of the postgres container from the docker-compose.yml.
#!/bin/bash
set -x
docker exec -u root jpg-$1 bash -c "chmod +x import_tpch_sf10.sh"
docker exec jpg-$1 bash -c "bash import_tpch_sf10.sh $1"
