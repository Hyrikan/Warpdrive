# This script executes an sql script in one o haris postgres docker databases
# Give the database number as first argument and the file as second
#! /bin/bash

psql postgresql://postgres:post1234@0.0.0.0:$15432/db1 -f $2
