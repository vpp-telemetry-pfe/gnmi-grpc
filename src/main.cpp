#include <iostream>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include "../proto/gnmi.grpc.pb.h"

using grpc::Status;
using grpc::ClientContext;
using gnmi::gNMI;
using gnmi::CapabilityRequest;
using gnmi::CapabilityResponse;

class GNMIServer final : public gNMI::Service {

  public:

    grpc::Status Capabilities(ClientContext* context, const 
        CapabilityRequest& request, CapabilityResponse* response) override
    {
      return Status::OK;
    }
};

int main (int argc, char* argv[]) {
}
