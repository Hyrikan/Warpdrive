#!/bin/bash

# start postgres
postgres -D /var/lib/postgresql/12/main -c "config_file=/etc/postgresql/12/main/postgresql.conf" >logfile 2>&1 & sleep 10

# start warpserver
./warpServer/build/pg_standalone_warpServer &

# Wait for any process to exit
wait -n

# Exit with status of process that exited first
exit $?
