# This script transfers lineitemsf1.csv from docker container jpg-1 to jpg-2 via netcat and measures the time it takes.
#!/bin/bash


echo "Starting netcat on jpg-2, listening on port 12345"
docker exec jpg-2 bash -c "netcat -l 12345 > /csvdir/lineitemsf1.csv &"
echo "Transfering lineitemsf1.csv from jpg-1 to jpg-2 via netcat and measuring the time"

starttime=$(date +%s%N)
docker exec jpg-1 bash -c "netcat -w 2 10.5.1.22 12345 < /csvdir/lineitemsf1.csv"
endtime=$(date +%s%N)

difftimeinseconds=$(echo "scale=9;($endtime-$starttime)/1000000000" | bc)

echo "Finished transfer"
echo "Result:"
echo "The raw transfer took $difftimeinseconds s."
#echo "Deleting both files"

#docker exec jpg-1 bash -c "rm /csvdir/lineitemsf1.csv"
#docker exec jpg-2 bash -c "rm /csvdir/lineitemsf1.csv"

echo "Finished"
