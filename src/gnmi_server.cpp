/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 smarttab: */

#include <iostream>
#include <memory>
#include <chrono>
#include <thread> //this_thread::sleep_for

#include <pthread.h>
#include <unistd.h>
#include <google/protobuf/repeated_field.h>

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

			// This only handles the case of a new RPC yet
			while (stream->Read(&request)) {

				// Replies with an error if there is no SubscriptionList field
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
							cout << "Received a STREAM SubscribeRequest" << endl;
              cout << request.DebugString() << endl;

							// Send first message: notification message
							BuildNotification(request, response);
              cout << "Sending the first update" << endl;
							cout << response.DebugString() << endl;
							stream->Write(response);
							response.clear_response();

							// Send second message: sync message
							response.set_sync_response(true);
              cout << "Sending sync response" << endl;
							cout << response.DebugString() << endl;
							stream->Write(response);
							response.clear_response();

							// Periodically updates paths that require SAMPLE updates
							
              // Associates each SAMPLE subscription to a chrono within a map
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

              // Main loop that sends updates for paths whose chrono has expired
              while(!context->IsCancelled()) {
                auto start = high_resolution_clock::now();
                // Initializes the updateList
                SubscribeRequest updateRequest(request);
                SubscriptionList* updateList(updateRequest.mutable_subscribe());
                updateList->clear_subscription();
                // Fills the list with Subscriptions whose chrono has expired
                for (int i=0; i<request.subscribe().subscription_size(); i++) {
                  unsigned long duration = duration_cast<nanoseconds> (
                      high_resolution_clock::now()-chronomap[i].second).count();
                  if (duration > chronomap[i].first.sample_interval()) {
                    chronomap[i].second = high_resolution_clock::now();
                    Subscription* sub = updateList->add_subscription();
                    sub->CopyFrom(chronomap[i].first);
                  }
                }
                // Sends a single message that updates all paths of the list
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

              cout << "Subscribe RPC call CANCELLED" << endl;
              break;
						}
					case SubscriptionList_Mode_ONCE:
            cout << "Received a ONCE SubscribeRequest" << endl;
            return Status(StatusCode::UNIMPLEMENTED,
                          grpc::string("ONCE mode not implemented yet"));
            break;
					case SubscriptionList_Mode_POLL:
            cout << "Received a POLL SubscribeRequest" << endl;
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
