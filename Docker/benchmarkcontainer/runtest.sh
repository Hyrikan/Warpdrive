# this script starts the test docker container with a volume bind on the benchmarkproject folder and runs mvn test for that project.
#!/bin/bash

docker start -a joel_benchmark # --mount source=benchmarkvolume,target=/benchmarkproject joel_test
