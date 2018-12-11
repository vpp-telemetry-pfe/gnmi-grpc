/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 smarttab: */

#include <iostream>
#include <memory>
#include <chrono>
#include <thread>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include "../proto/gnmi.grpc.pb.h"
#include "gnmi_encode.h"

using namespace grpc;
using namespace gnmi;
using namespace std;
using namespace chrono;
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

      // This only handles SubscriptionRequests with the subscription field.
      // Requests with poll and aliases fields are not handled yet.
      while (stream->Read(&request)) {

        if (!request.has_subscribe()) {
          // TODO: Return the error code in a SubscriptionRequest message
          // Ref: 3.5.1.1
          context->TryCancel();
          return Status(StatusCode::CANCELLED, grpc::string(
                "SubscribeRequest needs non-empty SubscriptionList"));
        }

        switch (request.subscribe().mode()) {
          case SubscriptionList_Mode_STREAM:
            {
              cout << request.DebugString() << endl;

              // Sends a Notification message that updates all Subcriptions
              BuildNotification(request, response);
              cout << response.DebugString() << endl;
              stream->Write(response);
              response.clear_response();

              // Sends a message that indicates that initial synchronization
              // has completed, i.e. each Subscription has been updated once
              response.set_sync_response(true);
              cout << response.DebugString() << endl;
              stream->Write(response);
              response.clear_response();

              /* Periodically updates paths that require SAMPLE updates
               * Note : There is only one Path per Subscription, but repeated
               * Subscriptions in a SubscriptionList, each Subscription can
               * have its own sample interval */

              vector<pair
                <Subscription, time_point<system_clock>>> chronomap;
              for (int i=0; i<request.subscribe().subscription_size(); i++) {
                Subscription sub = request.subscribe().subscription(i);
                switch (sub.mode()) {
                  case SAMPLE:
                    chronomap.emplace_back(sub, system_clock::now());
                    break;
                  default:
                    // TODO: Handle ON_CHANGE and TARGET_DEFINED modes
                    // Ref: 3.5.1.5.2
                    break;
                }
              }

              while(!context->IsCancelled()) {
                auto start = high_resolution_clock::now();

                SubscribeRequest updateRequest(request);
                SubscriptionList* updateList(updateRequest.mutable_subscribe());
                updateList->clear_subscription();

                for (int i=0; i<request.subscribe().subscription_size(); i++) {
                  unsigned long duration = duration_cast<nanoseconds> (
                      high_resolution_clock::now()-chronomap[i].second).count();
                  if (duration > chronomap[i].first.sample_interval()) {
                    chronomap[i].second = high_resolution_clock::now();
                    Subscription* sub = updateList->add_subscription();
                    sub->CopyFrom(chronomap[i].first);
                  }
                }

                if (updateList->subscription_size() > 0) {
                  BuildNotification(updateRequest, response);
                  cout << "Sending sampled update" << endl;
                  cout << response.DebugString() << endl;
                  stream->Write(response);
                  response.clear_response();
                }

                // Caps the loop at 5 iterations per second
                auto loopTime = high_resolution_clock::now() - start;
                this_thread::sleep_for(milliseconds(200) - loopTime);
              }

              break;
            }
          case SubscriptionList_Mode_ONCE:
            return Status(StatusCode::UNIMPLEMENTED,
                grpc::string("ONCE mode not implemented yet"));
            break;
          case SubscriptionList_Mode_POLL:
            return Status(StatusCode::UNIMPLEMENTED,
                grpc::string("POLL mode not implemented yet"));
            break;
          default:
            return Status(StatusCode::UNKNOWN,
                grpc::string("Unkown mode"));
        }
      }
      return Status::OK;
    }
};

void runServer()
{
  string server_address("0.0.0.0:50051");
  GNMIServer service;
  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  unique_ptr<Server> server(builder.BuildAndStart());
  cout << "Server listening on " << server_address << endl;
  server->Wait();
}

int main (int argc, char* argv[]) {
  runServer();
}
