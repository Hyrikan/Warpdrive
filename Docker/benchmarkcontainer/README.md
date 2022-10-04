This directory contains stuff to build a docker container, which executes maven tests from the benchmarkproject subdirectory.

To use it properly do these steps:
1. run "make initialize" to initialize the container. It will build the image, download the needed maven dependencies, create a volume to connect the project files with the container and run tests.
That first run will throw an exception, because the exported csv at one container was not send to the other and is therefore not present at the other container for import.
2. Now one has to call the lineitemtransfer scripts from the ../scripte/benchmarkscripte/ directory to send over the first csv files.
From now on, all files are present and just get overwritten when running tests again.
3. The warpDrive servers needs to get started manually. For that, one has to enter docker container
jpg-1 and run ./warpServer/build/pg_standalone_warpServer.
4. To subsequently rebuild the docker image, just run "make". It will also remove the old docker container and its attached anonymous volumes.
5. To now run the maven tests from the benchmarkproject, execute the "./runtest.sh" script.

You can Write more tests in the benchmarkproject, which automatically get included.

IMPORTANT: some tests are disabled by commenting out the @Test before the method header, due to some tests just failing ungracefully with low system memory.
