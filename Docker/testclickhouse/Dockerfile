# base clickhouse server
FROM clickhouse/clickhouse-server

# set commands to reduce interaction
ENV DEBIAN_FRONTEND noninteractive

# install java for jdbc
ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64/
ENV USER root

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        openjdk-8-jdk

RUN update-alternatives --set java /usr/lib/jvm/java-8-openjdk-amd64/jre/bin/java


# uncomment for clickhouse environment variables
#ENV CLICKHOUSE_DB=
#ENV CLICKHOUSE_USER=
#ENV CLICKHOUSE_DEFAULT_ACCESS_MANAGEMENT=
#ENV CLICKHOUSE_PASSWORD=
ENV CLASSPATH=/clickhouse-jdbc-bridge/drivers/

# Support for JDBC drivers
COPY ./clickhouse-jdbc-bridge /clickhouse-jdbc-bridge

# Add jdbc start script
COPY ./initscripts/startjdbc.sh /docker-entrypoint-initdb.d/startjdbc.sh

# Add TPC-H import scripts
COPY ./import/* /

# ports
EXPOSE 8123
EXPOSE 9000
EXPOSE 9004
EXPOSE 9005

# volumes (clickhouse data, logs) (is set in docker-compose)
#VOLUME /var/lib/clickhouse/

# Run jdbc bridge and then start clickhouse server
#CMD java -jar /clickhouse-jdbc-bridge/clickhouse-jdbc-bridge-2.0.7-shaded.jar & clickhouse start
