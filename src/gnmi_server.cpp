/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 smarttab: */

#include <iostream>
#include <memory>
#include <google/protobuf/repeated_field.h>

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
using google::protobuf::RepeatedPtrField;

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
							std::cout << "Received a request" << std::endl;
							/*  Build a new Notificiation Protobuf Message */
              Notification* notification = new Notification();

              notification->set_timestamp(std::time(0));

              if (request.subscribe().has_prefix()) {
                Path* prefix = notification->mutable_prefix();
                prefix->set_target(request.subscribe().prefix().target());
              }

              // TODO : Notification.alias

              /* Embedded Update message inside Notification message */
              RepeatedPtrField<Update>* updateL = notification->mutable_update();

							Update * update = updateL->Add();
              Path* path = update->mutable_path();
              PathElem* pathElem = path->add_elem();
              pathElem->set_name("path_elem_name");
              TypedValue* val = update->mutable_val();
              val->set_string_val("Test message");
              update->set_duplicates(0);

              // TODO: Notification.delete

              // Notification.atomic
              notification->set_atomic(false);

              // Send first message: notification message
							std::cout << notification->DebugString() << std::endl;
              stream->Write(response);

              // Send second message: sync message
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
