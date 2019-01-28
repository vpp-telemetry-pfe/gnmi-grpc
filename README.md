# GNMI gRPC Server

## Description

This repo provide a C++ gRPC server based on [gNMI specification](https://github.com/openconfig/reference/blob/master/rpc/gnmi/gnmi-specification.md) to stream telemetry informations from vpp.

A typical setup would make gNMI server work with a data collector supporting gNMI such as [pipeline-gnmi](https://github.com/cisco-ie/pipeline-gnmi) or telegraf with a gnmi plugin.

The data collector make subscription request to gnmi server to receive updates on a regular interval about metrics.

Our server creates telemetry messages by filling timestamp field at nanosecond precision. Thus our system has a max time limit which expires 292 years after epoch (1970).
`9223372036854775806/10^9/60/60/24/365=292.47120867753601617199`

## Requirements

* grpc version > 1.9.x
* protobuf >= 3.0 & compatible with gRPC

## Install

On your VPP instance:

Install grpcpp:
---------------

In order to run gRPC server with pipeline-gnmi you will need a working version of gRPC cplusplus with google protocol buffer version 3.
Please read the following instructions [https://github.com/grpc/grpc/blob/master/BUILDING.md ](https://github.com/grpc/grpc/blob/master/BUILDING.md ).
You will need to compile grpc and install third_parties/protobuf to run compaitble versions of protobuf and gRPC.

Compile and run:
----------------

```
make
./build/gnmi_server -f #no encryption, no authentication
```

Use with a data collector:
--------------------------

* Clone pipeline-gnmi project

`git clone https://github.com/cisco-ie/pipeline-gnmi.git`

* Run pipeline with pipeline.conf file provided in gnmi-grpc repository:

`./bin/pipeline -c <path to pipeline.conf>`

## Running the docker scenario

We provided a basic docker scenario as a demonstration. It uses:

* a VPP instance built on ubuntu bionic with grpcpp 1.12.0;
* a pipeline-gnmi instance built on alpine 3.8 collecting data from gNMI server
* a Influxdb instance to store telemetry informations received from pipeline
* a Chronograf instance to visualize telemetry informations, use dashboard, ...

```
make clean
docker-compose build #build vpp1
docker-compose up
```

See chronograf : [http://localhost:8888](http://localhost:8888)

Use Influx client to print timestamp in RFC3339:

```
./influx --precision rfc3339
> connect localhost:8086
> show databases
```

## Further notes

* gRPC has switched from grpc++ to grpcpp directory in header file. Thus, you need a recent version of gRPC.

* This repo notably upgrades the [vpp_prometheus_export HTTP1.1 server](https://github.com/FDio/vpp/blob/3620e61dece82781f0073a2908dfcb24724712a9/src/vpp/app/vpp_prometheus_export.c).
