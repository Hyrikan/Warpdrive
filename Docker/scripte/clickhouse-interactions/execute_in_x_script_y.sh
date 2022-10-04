#!/bin/bash
# this script connects to one of joels clickhouse containers and executes an sql file in it.

clickhouse-client -h "10.5.1.3$1" --multiquery < "$2"
