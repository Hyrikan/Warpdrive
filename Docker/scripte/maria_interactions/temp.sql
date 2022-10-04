create function IF NOT EXISTS envar returns string soname 'ha_connect.so';
select envar('CLASSPATH');
