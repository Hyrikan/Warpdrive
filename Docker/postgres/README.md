This postgresql docker container hosts postgresql server 12 and installs the warpFDW extension.

The Makefile builds the container image from the Dockerfile and gives it the tag "joel_postgres".

The start.sh and end.sh scripts start and end a lone joel_postgres container based of the docker-compose.yml in this directory.

To import test data into the postgres instance, include the volume with testdata generated with the tpch_generator container and run the import scripte.
