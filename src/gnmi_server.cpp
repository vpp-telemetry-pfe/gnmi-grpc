/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 smarttab: */

#include <iostream>
#include <memory>
#include <chrono>
#include <thread> //std::this_thread::sleep_for

#include <pthread.h>
#include <unistd.h>
#include <google/protobuf/repeated_field.h>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include "../proto/gnmi.grpc.pb.h"
#include "gnmi_encode.h"

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
using gnmi::Subscription;
using gnmi::SubscriptionList;
using gnmi::CapabilityRequest;
using gnmi::CapabilityResponse;
using gnmi::SubscriptionList_Mode_ONCE;
using gnmi::SubscriptionList_Mode_POLL;
using gnmi::SubscriptionList_Mode_STREAM;
using gnmi::SAMPLE;
using gnmi::ON_CHANGE;
using gnmi::TARGET_DEFINED;
using google::protobuf::RepeatedPtrField;

using namespace std::chrono;

struct streamRequest {
  shared_ptr<SubscribeRequest> request;
  shared_ptr<ServerReaderWriter<SubscribeResponse, SubscribeRequest>> stream;
};

void buildNotification( // Const arguments ?
    shared_ptr<SubscribeRequest> request, SubscribeResponse& response)
{
  Notification *notification = response.mutable_update();
  milliseconds ts;

  /*  Get non-monotic timestamp since epoch in msecs when data is
    *  generated */
  ts = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
  notification->set_timestamp(ts.count());

  // Prefix for all counter paths
  if (request->subscribe().has_prefix()) {
    Path* prefix = notification->mutable_prefix();
    prefix->set_target(request->subscribe().prefix().target());
  }

  // TODO : Notification.alias

  // repeated Notification.update
  for (int i=0; i<request->subscribe().subscription_size(); i++) {
    Subscription sub = request->subscribe().subscription(i);
    RepeatedPtrField<Update>* updateL = 
      notification->mutable_update();
    Update* update = updateL->Add();

    /* If a directory path has been provided in the request, we must
    * get all the leaves of the file tree. */
    Path* path = update->mutable_path();
    path->CopyFrom(sub.path());
    /* UnixtoGnmiPath("/err/vmxnet3-input/Rx no buffer error", path);
    PathElem* pathElem = path->add_elem();
    pathElem->set_name("path_elem_name"); */

    // TODO: Fetch the value from the stat_api instead of hardcoding one
    TypedValue* val = update->mutable_val();
    val->set_string_val("Test message number " + std::to_string(i));

    update->set_duplicates(0);
  }

  // TODO: Notification.delete

  // Notification.atomic
  notification->set_atomic(false);
}

void* streamRoutine(void* threadarg)
{
  std::cout << "Thread launched" << std::endl;
  streamRequest* streamReq = static_cast<struct streamRequest*>(threadarg);
  if (streamReq == NULL) {
    pthread_exit(NULL);
  }
  std::cout << streamReq->request->DebugString() << std::endl;

  // Creates the map that links SAMPLE subscriptions to their respective chrono
  std::vector<std::pair<Subscription, time_point<system_clock>>> chronomap;
  for (int i=0; i<streamReq->request->subscribe().subscription_size(); i++) {
    Subscription sub = streamReq->request->subscribe().subscription(i);
    switch (sub.mode()) {
      case SAMPLE:
      {
        chronomap.emplace_back(sub, system_clock::now());
        break;
      }
      default:
      { 
        // TODO: Handle ON_CHANGE and TARGET_DEFINED modes
        break;
      }
    }
  }

  // Main loop that checks every chrono of the map, capped at 5 loops / second
  while(true) { // TODO: Change this condition
    auto start = system_clock::now();
    // Iterate through chronomap, send messages if needed and reset chronos
    for (auto it = chronomap.begin(); it != chronomap.end(); it++) {
      if (system_clock::now() - it->second >
          milliseconds(it->first.sample_interval()*1000))
      {
        SubscribeResponse response;
        buildNotification(streamReq->request, response);
        streamReq->stream->Write(response);
        it->second = system_clock::now();
      }
    }
    duration<double> duration = system_clock::now() - start;
    std::this_thread::sleep_for(milliseconds(200) - duration);
  }

  pthread_exit(NULL);
}

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
							std::cout << "Received a STREAM SubscribeRequest" << std::endl;

							buildNotification(
							    std::make_shared<SubscribeRequest>(request), response);

							// Send first message: notification message
							std::cout << response.DebugString() << std::endl;
							stream->Write(response);
							response.clear_update();

							// Send second message: sync message
							response.set_sync_response(true);
							std::cout << response.DebugString() << std::endl;
							stream->Write(response);
							response.clear_update();

              // Launch dedicated thread to keep streaming updates
              streamRequest streamRequest = {
                .request = std::make_shared<SubscribeRequest>(request),
                .stream = std::make_shared<ServerReaderWriter<
                  SubscribeResponse, SubscribeRequest>>(*stream)
              };
							pthread_t thread;
							if (pthread_create(
							      &thread, NULL, streamRoutine, (void *) &streamRequest)) {
                std::cout << "Error launching thread" << std::endl;
              }
              pthread_join(thread, NULL);
            break;
						}
					case SubscriptionList_Mode_ONCE:
						{
							/* TODO: Same as above but no need for a thread to handle it */
							std::cout << "Received a ONCE SubscribeRequest" << std::endl;
							return Status(StatusCode::UNIMPLEMENTED,
														grpc::string("ONCE mode not implemented yet"));
							break;
						}
					case SubscriptionList_Mode_POLL:
						{
							std::cout << "Received a POLL SubscribeRequest" << std::endl;
							return Status(StatusCode::UNIMPLEMENTED,
														grpc::string("POLL mode not implemented yet"));
							break;
						}
					default:
						return Status(StatusCode::UNIMPLEMENTED,
													grpc::string("Unkown mode"));
				}
			}
			return Status::OK;
		}
};

void runServer()
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
	runServer();
}
