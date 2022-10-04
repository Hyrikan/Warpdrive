# This script executes an sql file in one of haris mysql docker databases
# Just give the number of the database as first argument and the file as second
#! /bin/bash

mysql --host=0.0.0.0 --port=$13306 --user=mariadb --database=mdb --password=maria1234 < $2
