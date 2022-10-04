# Warpdrive
Framework for generic and efficient inter database data transfer

## Installation
The Warpdrive directory contains all source code for compiling the Warpdrive. The Makefile assumes Ubuntu 20.04 and GCC for compilation. Additionally, an installation script just copies the compiled libraries into the Ubuntu 20.04 standart library directories.

## Testing
The Docker directory contains docker images used for testing the Warpdrive against JDBC, CSV and foreign data wrapper data transfer on PostgreSQL and Clickhouse. Docker and Docker-Compose is required for testing. To build the Docker images, run make in the Docker directory. To start the testing set up, run "docker-compose up" or use the start and stop scripts. To start testing, go into the benchmark directory and continue with instructions provided there. There are additional README files inside each directory to give more detailed information and usage descriptions.