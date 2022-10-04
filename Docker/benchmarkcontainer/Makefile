mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))

all: build createDockerVolume removeOldContainer
	docker run --name joel_benchmark --network joeldb-net --mount source=benchmarkvolume,target=/benchmarkproject joel_test

initialize: build createDockerVolume
	docker run --name joel_benchmark --network joeldb-net --mount source=benchmarkvolume,target=/benchmarkproject joel_test

build:
	docker build -t joel_test .

createDockerVolume:
	docker volume create --driver local --opt type=none --opt device=$(mkfile_dir)benchmarkproject --opt o=bind benchmarkvolume

removeOldContainer:
	docker container rm -v joel_benchmark
