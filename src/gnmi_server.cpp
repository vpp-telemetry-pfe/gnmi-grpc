#include <iostream>
#include <memory>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include "../proto/gnmi.grpc.pb.h"

using grpc::Status;
using grpc::StatusCode;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using gnmi::gNMI;
using gnmi::Error;
using gnmi::Notification;
using gnmi::GetRequest;
using gnmi::SetRequest;
using gnmi::GetResponse;
using gnmi::SetResponse;
using gnmi::SubscribeRequest;
using gnmi::SubscribeResponse;
using gnmi::SubscriptionList;
using gnmi::CapabilityRequest;
using gnmi::CapabilityResponse;
using gnmi::SubscriptionList_Mode_ONCE;
using gnmi::SubscriptionList_Mode_POLL;
using gnmi::SubscriptionList_Mode_STREAM;

class GNMIServer final : public gNMI::Service
{
  public:

    Status Capabilities(ServerContext* context, 
        const CapabilityRequest* request, CapabilityResponse* response)
    {
      return Status(StatusCode::UNIMPLEMENTED,
          grpc::string("'Capabilities' not implemented yet"));
    }

    Status Get(ServerContext* context, 
        const GetRequest* request, GetResponse* response)
    {
      return Status(StatusCode::UNIMPLEMENTED,
          grpc::string("'Get' method not implemented yet"));
    }

    Status Set(ServerContext* context, 
        const SetRequest* request, SetResponse* response)
    {
      return Status(StatusCode::UNIMPLEMENTED,
          grpc::string("'Set' method not implemented yet"));
    }

    Status Subscribe(ServerContext* context, 
        ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream)
    {      
      SubscribeRequest request;
      SubscribeResponse response;
      // This only handles the case of a new RPC
      while (stream->Read(&request)) {
        // Replies with an error if there is no SubscriptionList field
        if (!request.has_subscribe()) {
          //TODO: Return the error code in a SubscriptionRequest message
          stream->Write(response);
          context->TryCancel();
          return Status(StatusCode::CANCELLED, grpc::string(
                "A SubscribeRequest needs a non-empty SubscriptionList"));
        }
        // Handles a well-formed request (i.e. with a SubscriptionList field)
        switch (request.subscribe().mode()) {
          case SubscriptionList_Mode_STREAM:
            //TODO: Handle STREAM mode
            std::unique_ptr<Notification> update(new Notification());
            update->set_timestamp(1);
            //update->set
            //response.set_allocated_update(gnmi::Notification* update);
            //server->Write(...);
            break;
          default:
            return Status(StatusCode::UNIMPLEMENTED,
                grpc::string("POLL and ONCE modes not implemented yet"));
        }
      }

      return Status::OK;
    }
};

void RunServer()
{
  std::string server_address("0.0.0.0:50051");
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
