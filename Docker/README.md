This directory contains all the docker stuff Joel needs in his bachelor thesis.

The mariadb, clickhouse and postgres directories contain Dockerfiles and sources to create the appropriate docker images.

The tpc-h generator directory contains a dockerfile to generate a docker volume with tpc-h test data.

The script directory contains scripts to interact with the docker containers. Especially the import
scripts are important to import tpc-h data into the databases from the previous generated tpc-h test data volume.
Care should be taken to only run import scripts once, otherwise the container contains the imported data more than once!

HOW TO USE:

call "make" in this directory to build the three docker containers used in the docker-compose. Go into the tpch_generator directory
and use "make" to generate the volume with 1gb and 10gb tpch test data. (takes a while)
Now call docker-compose up in this directory to bring the containers up.
Now go into the script directory and use the import scripts in each subdirectory to import the test data into the databases of the containers.
Now have fun :)
