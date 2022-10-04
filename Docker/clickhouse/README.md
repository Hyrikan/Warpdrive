This is the docker image for benchmarking and further integration of the bachelor thesis...
call "make" in clickhouse-jdbc-bridge/ to compile the image. Its called joel_clickhouse_allinone.
The normal Dockerfile is for a bridge only docker image, the Makefile uses the all_in_one.Dockerfile for 
a combination of clickhouse and bridge in one container.
