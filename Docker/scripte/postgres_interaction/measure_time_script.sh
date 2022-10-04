# this script should measure the time it takes for a postgres docker container to export it's lineitem table into a csv file.
#!/bin/bash

starttime=$(date +%s%N)
sleep 1








endtime=$(date +%s%N)
diff=$(($endtime-$starttime))
inseconds=$(echo "scale=9;($endtime-$starttime)/1000000000" | bc)
milli=$(((($endtime-$starttime)% 1000000000)/1000000))
micro=$(((($endtime-$starttime)% 1000000)/1000))

echo "Here the results"
echo "It took $inseconds s seconds."
