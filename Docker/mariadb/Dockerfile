# base image. Same as in Haris maria docker.
FROM mariadb:10.5-focal

# set commands to reduce interaction
ENV DEBIAN_FRONTEND noninteractive

# setting password as environment variable.
#ENV MYSQL_ROOT_PASSWORD="maria1234"

# -------------------------------------- Haris mariadb stuff (odbc, import scripts..) -----------------------------------------
# install prerequisites
RUN apt-get update && apt install openjdk-8-jdk -y && export JAVA_HOME=/usr/lib/jvm/java-1.8.0-openjdk-amd64
RUN apt-get remove --purge mariadb-server-10.5 -y && apt-get autoremove -y && apt-get autoclean -y
RUN apt-get update && apt-get install mariadb-server-10.5 mariadb-plugin-connect -y
RUN apt-get update && apt-get install libodbc1 unixodbc-dev libssl-dev odbc-postgresql curl alien -y

# comment out a few problematic configuration values
RUN	find /etc/mysql/ -name '*.cnf' -print0 \
		| xargs -0 grep -lZE '^(bind-address|log|user\s)' \
		| xargs -rt -0 sed -Ei 's/^(bind-address|log|user\s)/#&/';
# don't reverse lookup hostnames, they are usually another container
RUN	echo '[mysqld]\nskip-host-cache\nskip-name-resolve' > /etc/mysql/conf.d/docker.cnf

# set classpath variable for jars
ENV CLASSPATH "/usr/lib/mysql/plugin/JavaWrappers.jar:/usr/lib/mysql/plugin/JdbcInterface.jar:/tmp/mariadb-java-client-3.0.4.jar:/tmp/postgresql-42.3.3.jar"

# copy plugin files and scripts
COPY ./jars/* /usr/lib/mysql/plugin/
COPY ./lib/* /tmp/
COPY ./connect.cnf /etc/mysql/mariadb.conf.d/connect.cnf
RUN chmod 0444 /etc/mysql/mariadb.conf.d/connect.cnf
COPY privileges.sql /docker-entrypoint-initdb.d/

# copy tpc-h import scripts
COPY import/create_tpch_sf1.sql /create_tpch_sf1.sql
COPY import/import_tpch_sf1.sql /import_tpch_sf1.sql
COPY import/indexes_tpch_sf1.sql /indexes_tpch_sf1.sql


# set persistent data volume (will be set in docker-compose)
#VOLUME /var/lib/mysql

# expose port
EXPOSE 3306

# run mariadb server command
CMD ["mysqld"]



