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
using gnmi::Update;
using gnmi::TypedValue;
using gnmi::Path;
using gnmi::PathElem;
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
      // This only handles the case of a new RPC yet
      while (stream->Read(&request)) {
        // Replies with an error if there is no SubscriptionList field
        if (!request.has_subscribe()) {
          //TODO: Return the error code in a SubscriptionRequest message
          //stream->Write(response);
          context->TryCancel();
          return Status(StatusCode::CANCELLED, grpc::string(
                "A SubscribeRequest needs a non-empty SubscriptionList"));
        }
        // Handles a well-formed request (i.e. with a SubscriptionList field)
        switch (request.subscribe().mode()) {
          case SubscriptionList_Mode_STREAM:
            {
              auto notification = std::make_unique<Notification>();
              // Notification.timestamp
              notification->set_timestamp(std::time(0));
              // Notification.prefix
              if (request.subscribe().has_prefix()) {
                auto prefix = std::make_unique<Path>();
                prefix->set_target(request.subscribe().prefix().target());
                notification->set_allocated_prefix(prefix.get());
              }
              // Notification.alias
              //TODO
              // Notification.update
              auto update = std::make_unique<Update>();
              update->set_duplicates(0);
              // Notification.upate.path
              auto path = std::make_unique<Path>();
              auto pathElem = std::make_unique<PathElem>();
              pathElem->set_name("path_elem_name");
              /**
               * TODO: Add pathElem to path before adding path to update
               * path->add_elem();
               */
              update->set_allocated_path(path.get());
              // Notification.update.val
              auto val = std::make_unique<TypedValue>();
              val->set_string_val("Test message");
              update->set_allocated_val(val.get());
              // Notification.delete
              //TODO
              // Notification.atomic
              notification->set_atomic(false);

              // First message: notification message
              response.set_allocated_update(notification.get());
              stream->Write(response);
              // Second message: sync message
              response.clear_update();
              response.set_sync_response(true);
              stream->Write(response);
              break;
            }
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
