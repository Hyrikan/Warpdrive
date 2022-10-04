This directory contains everything to populate a docker volume with tpch test data, which can be further utilized by other docker containers.

Just call "make" and afterwards "docker-compose up". It may take a while to generate the 1GB and 10GB of data.

The resulting docker volume containing the data is called "tpch_generator_tpch-data".

Calling "make" builds a docker image called "tpcgen" and copies the run.sh script into it. The main command of the image runs the run.sh script, which 
populates the /data directory with 1GB and 10GB of tpch test data.
Calling docker-compose up starts the service tpch_generator with the above image and creates tpch test data in a volume called "tpch_generator_tpch-data".
This volume can be used by other docker containers to access tpch test data.
