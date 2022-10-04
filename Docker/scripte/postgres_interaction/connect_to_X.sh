# This script connects to one of the postgres containers from the docker-compose.
# You can connect to the n-th postgres database by giving the number n as parameter.
# It also kinda stores the command for connecting to a postgres database.
#! /bin/bash

psql postgresql://postgres:post1234@0.0.0.0:$15432/db1
