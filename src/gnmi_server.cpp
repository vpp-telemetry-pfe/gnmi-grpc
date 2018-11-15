#include <iostream>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include "../proto/gnmi.grpc.pb.h"

using grpc::Status;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using gnmi::gNMI;
using gnmi::GetRequest;
using gnmi::GetResponse;
using gnmi::SetRequest;
using gnmi::SetResponse;
using gnmi::SubscribeRequest;
using gnmi::SubscribeResponse;
using gnmi::CapabilityRequest;
using gnmi::CapabilityResponse;

class GNMIServer final : public gNMI::Service {

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

int main (int argc, char* argv[]) {
}
