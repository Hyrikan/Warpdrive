# base image is ubuntu 20.04
FROM ubuntu:focal

# set commands to reduce interaction requirements
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update

# ---------------- INSTALL APACHE ARROW CGLIB ---------------------------------------------------------------------------------
# run all the install commands from https://arrow.apache.org/install/#C++_and_GLib_(C)_Packages_for_Debian_GNU/Linux,_Ubuntu,_CentOS_and_Amazon_Linux
RUN apt install -qy -V ca-certificates lsb-release wget
RUN ["sh","-c","wget https://apache.jfrog.io/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb"]
RUN ["sh","-c","apt install -qy -V ./apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb"]
RUN apt update
RUN apt install -qy -V libarrow-dev
RUN DEBIAN_FRONTEND=noninteractive TZ=Europe apt install -qy -V libarrow-glib-dev
RUN apt install -qy -V libarrow-dataset-dev
RUN apt install -qy -V libarrow-flight-dev

# ---------------- INSTALL POSTGRESQL ----------------------------------------------------------------------------------------
# install command
RUN apt-get install -qy --no-install-recommends \
	    postgresql

# continue as postgres user, because that is the root for postgres	    
USER postgres

# try adding environment variables
RUN echo "LD_LIBRARY_PATH='/usr/lib/warpdrive:/usr/include/ucx:/usr/lib:/usr/lib/ucx:/usr/lib/postgresql/12/lib:$LD_LIBRARY_PATH'" > /etc/postgresql/12/main/environment

# set password for postgres root user
RUN /etc/init.d/postgresql start && psql --command "ALTER USER postgres WITH PASSWORD 'post1234';" \
	    && createdb -O postgres db1 \
	    && /etc/init.d/postgresql stop \
	    && echo "host all  all    0.0.0.0/0  md5" >> /etc/postgresql/12/main/pg_hba.conf \
	    && echo "listen_addresses='*'" >> /etc/postgresql/12/main/postgresql.conf

# expose postgres Port
EXPOSE 5432

# set volume for persistent data (set in docker-compose)
#VOLUME /var/lib/postgresql/data

# set more volumes like in Haris postgresql (not sure what exactly gets stored there) (activate if i need some. Better set them in docker-compose)
#VOLUME  ["/etc/postgresql", "/var/log/postgresql", "/var/lib/postgresql"]

# set postgres directory for path
ENV PATH /usr/lib/postgresql/12/bin:$PATH

# set other paths
ENV C_INCLUDE_PATH /usr/include/postgresql:$C_INCLUDE_PATH
ENV CPLUS_INCLUDE_PATH=/usr/include/postgresql:$CPLUS_INCLUDE_PATH
ENV LD_LIBRARY_PATH=/usr/lib/warpdrive:/usr/include:/usr/include/ucx:/usr/lib:/usr/lib/ucx:/usr/lib/postgresql/12/lib:$LD_LIBRARY_PATH
ENV LIBRARY_PATH=/usr/lib/postgresql/12/lib:$LIBRARY_PATH

# Copy over the import scripts for the tpc-h testdata
COPY import/create_tpch_sf1.sql /create_tpch_sf1.sql
COPY import/import_tpch_sf1.sh /import_tpch_sf1.sh

COPY import/create_tpch_sf10.sql /create_tpch_sf10.sql
COPY import/import_tpch_sf10.sh /import_tpch_sf10.sh

# ------------------ INSTALL other stuff from Hari (CartoDB FDW, MariaDB connector) -----------------------------------------------

# set user back to root for installs and more
USER root

# install git, c compiler...
RUN apt-get update
RUN apt-get install -qy git
RUN apt-get install -qy make clang
RUN apt-get install -qy gcc

# install postgresql server dev packages for extension compilation
RUN apt-get install -qy postgresql-server-dev-12 postgresql-common libpam-dev libselinux-dev libedit-dev libxslt1-dev libkrb5-dev zlib1g-dev

# install prerequisites
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        libc-dev \
        cmake \
        ca-certificates \
        odbc-postgresql \
        unixodbc-dev \
        libssl-dev

# install cartodb
RUN git clone https://github.com/CartoDB/odbc_fdw.git && cd odbc_fdw && make && make install

# install mariadb connector
RUN git clone https://github.com/MariaDB/mariadb-connector-odbc.git
RUN mkdir build && cd build && cmake ../mariadb-connector-odbc/ -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCONC_WITH_UNIT_TESTS=Off -DCMAKE_INSTALL_PREFIX=/usr/local -DWITH_SSL=OPENSSL \
    && cmake --build . --config RelWithDebInfo \
    && make install

# copy config for maria odbc
COPY config/mariadb.ini /
RUN cat mariadb.ini >> /etc/odbcinst.ini

# ---------------- INSTALL PG2ARROW -------------------------------------------------------------------------------------------

# get pg2arrow from git
RUN git clone https://github.com/heterodb/pg2arrow.git

# change workdir for make
WORKDIR /pg2arrow

# make
RUN make PG_CONFIG=/usr/lib/postgresql/12/bin/pg_config

# install pg2arrrow
RUN make install

# ---------------- INSTALL click fdw --------------------------------------------------------------------------------------------

# install dependency
RUN apt-get update && apt-get install -qy --no-install-recommends uuid libcurl4-openssl-dev

# clone the git
WORKDIR /
RUN git clone https://github.com/adjust/clickhouse_fdw.git

# more dependencies
RUN apt-get install -qy --no-install-recommends make musl-dev gcc g++ cmake curl 

# install rerequisites and fdw
RUN mkdir /clickhouse_fdw/build
RUN sed -i 48,49's/ gcc/ PRIVATE gcc/' /clickhouse_fdw/src/clickhouse-cpp/clickhouse/CMakeLists.txt
WORKDIR /clickhouse_fdw/build
RUN cmake ..
RUN make && make install

# ---------------- Install JDBC fdw and driver ------------------------------------------------------------------------------------

# install java
WORKDIR /
RUN apt-get install -yq --no-install-recommends openjdk-8-jdk 

# clone git
RUN git clone https://github.com/atris/JDBC_FDW

# create symlink for jvm
RUN ln -s /usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/server/libjvm.so /usr/lib64/libjvm.so
RUN ln -s /usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/server/libjvm.so /usr/lib/libjvm.so

# copy over jdbc driver
COPY jdbcdriver /JDBC_FDW/jdbcdriver

# build
WORKDIR /JDBC_FDW
RUN make USE_PGXS=1 clean
RUN make USE_PGXS=1
RUN make USE_PGXS=1 install
RUN ldconfig

# ------------------ Install prerequisites for csv file transfer --------------------------------------------------------------------------------------

# create working directory where postgresuser can create csv files
RUN mkdir /csvdir
RUN chmod a+rwx /csvdir

# Get netcat for plain and fast files transfer
RUN apt-get install -qy netcat

# ------------------ Install clickhouse-client for libraries needed with clickhouse jdbc jar -----------------------------------------------

RUN apt-get update && apt-get install -qy clickhouse-client


# ---------------- INSTALL Warpdrive ----------------------------------------------------------------------------------------------

# copy warpdrive directory into the container
COPY warpdrive /warpdrive

# set working directory
WORKDIR /warpdrive

# make scripts executable
RUN chmod +x installscript.sh

# run install script (copies libs and headers around)
RUN ["/bin/bash", "-c", "./installscript.sh"]

#change workdir back to default
WORKDIR /

# Try setting ld environment
ENV LD_LIBRARY_PATH=/usr/lib/warpdrive:/usr/include:/usr/include/ucx:/usr/lib:/usr/lib/ucx:/usr/lib/postgresql/12/lib:$LD_LIBRARY_PATH
RUN ldconfig

# add additional package needed
RUN apt-get install -qy libnuma-dev

# ---------------- INSTALL Bachelor FDW ------------------------------------------------------------------------------------------------

# copy foreign data wrapper code into the image
COPY pg_warpFDW /pg_warpFDW

# set working directory
WORKDIR /pg_warpFDW

# make scripts executable
RUN chmod +x installcommand.sh
RUN chmod +x compilecommand.sh

# run install scripts
RUN ["/bin/bash", "-c", "./compilecommand.sh"]
RUN ["/bin/bash", "-c", "./installcommand.sh"]

#change workdir back to default
WORKDIR /

# ------------------ Install Postgres warpServer prototype -----------------------------------------
ENV test=test2
# copy relevant files over
COPY warpServer /warpServer

# create build environment with cmake
RUN mkdir /warpServer/build
WORKDIR /warpServer/build
RUN cmake ..

# compile target server
RUN cmake --build . --target pg_standalone_warpServer

# Expose port for warpServer
EXPOSE 13337

# change workdir back to default
WORKDIR /

# ------------------ START POSTGRES SERVER --------------------------------------------------------------------------------------

# set user back to postgres for running the postgres server
USER postgres

# set main process
CMD ["postgres","-D","/var/lib/postgresql/12/main","-c","config_file=/etc/postgresql/12/main/postgresql.conf"]

# ------------------ Alternative start ---------------------------------
# Copy wrapper start script over
#COPY wrapper_script_docker.sh /wrapper_script_docker.sh

# Start
#CMD ./wrapper_script_docker.sh
