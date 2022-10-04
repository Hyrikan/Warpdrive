# This script connects to one of haris mysql docker databases
# Just give the number of the database as argument
#! /bin/bash

mysql --host=0.0.0.0 --port=$13306 --user=mariadb --database=mdb --password=maria1234
#mysql --host=0.0.0.0 --port=$13306 --user=mariadb --password=maria1234
