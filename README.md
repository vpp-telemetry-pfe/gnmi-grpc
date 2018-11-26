# GNMI gRPC Server

## Description

This repo aims at upgrading the vpp_prometheus_export HTTP1.1 server, whose original implementation can be found here:

[ https://github.com/FDio/vpp/blob/3620e61dece82781f0073a2908dfcb24724712a9/src/vpp/app/vpp_prometheus_export.c ](https://github.com/FDio/vpp/blob/3620e61dece82781f0073a2908dfcb24724712a9/src/vpp/app/vpp_prometheus_export.c)

VPP has an implementation which uses HTTP polling to collect telemetry counters with prometheus data collector.

The idea is to use pipeline-gnmi data collector to create telemetry subscriptions with gRPC server and then to stream these informations to the data collector.

## Install

In order to run gRPC server with pipeline-gnmi you will need a working version of gRPC cplusplus with google protocol buffer version 3.
Please read the following instructions [https://github.com/grpc/grpc/blob/master/BUILDING.md ](https://github.com/grpc/grpc/blob/master/BUILDING.md ).
You will need to compile grpc and install third_parties/protobuf to run compaitble versions of protobuf and gRPC.

* Clone pipeline-gnmi project

`git clone https://github.com/cisco-ie/pipeline-gnmi.git`

* Clone this repository if not done yet:

`git clone https://github.com/vpp-telemetry-pfe/gnmi-grpc.git`

## Run

* Run an gnmi_server:

`./build/gnmi_server`

* Run pipeline with pipeline.conf file provided in gnmi-grpc repository:

`./bin/pipeline -c <path to pipeline.conf>`
