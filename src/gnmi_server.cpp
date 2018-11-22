#include <iostream>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include "../proto/gnmi.grpc.pb.h"

using grpc::Status;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using gnmi::gNMI;
using gnmi::GetRequest;
using gnmi::SetRequest;
using gnmi::GetResponse;
using gnmi::SetResponse;
using gnmi::SubscribeRequest;
using gnmi::SubscribeResponse;
using gnmi::CapabilityRequest;
using gnmi::CapabilityResponse;

class GNMIServer final : public gNMI::Service
{
  public:

    Status Capabilities(ServerContext* context, 
        const CapabilityRequest* request, CapabilityResponse* response)
    {
      return Status::OK;
    }

    Status Get(ServerContext* context, 
        const GetRequest* request, GetResponse* response)
    {
      return Status::OK;
    }

    Status Set(ServerContext* context, const SetRequest* request, SetResponse* response)
    {
      return Status::OK;
    }

    Status Subscribe(ServerContext* context, ServerReaderWriter< SubscribeResponse, SubscribeRequest>* stream)
    {
      return Status::OK;
    }
};

void RunServer()
{
  std::string server_address("0.0.0.0:57334");
  GNMIServer service;
  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main (int argc, char* argv[]) {
  RunServer();
}
