# This script transfers lineitemsf1.csv from docker container jpg-1 to jpg-2 via netcat and measures the time it takes.
#!/bin/bash


echo "Starting netcat on jpg-1, listening on port 12345"
docker exec jpg-1 bash -c "netcat -l 12345 > /csvdir/click_lineitemsf1.csv &"
echo "Transfering lineitemsf1.csv from jclick-1 to jpg-1 via netcat and measuring the time"

starttime=$(date +%s%N)
docker exec jclick-1 bash -c "netcat -w 2 10.5.1.21 12345 < /var/lib/clickhouse/data/default/click_export_lineitemsf1/data.CSV"
endtime=$(date +%s%N)

difftimeinseconds=$(echo "scale=9;($endtime-$starttime)/1000000000" | bc)

echo "Finished transfer"
echo "Result:"
echo "The raw transfer took $difftimeinseconds s."

echo "Finished"
