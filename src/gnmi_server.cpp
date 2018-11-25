/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 smarttab: */

#include <iostream>
#include <memory>
#include <chrono>
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

using namespace std::chrono;

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
					context->TryCancel();
					return Status(StatusCode::CANCELLED, grpc::string(
												"SubscribeRequest needs non-empty SubscriptionList"));
				}
				switch (request.subscribe().mode()) {
					case SubscriptionList_Mode_STREAM:
						{
							std::cout << "Received a STREAM subscription req" << std::endl;

							/*  Build a Notification Protobuf Message to communicate counters
							 *  updates. */
							Notification *notification = response.mutable_update();
							milliseconds ts;

							/*  Get non-monotic timestamp since epoch in msecs when data is
							 *  generated */
							ts = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
							notification->set_timestamp(ts.count());

							/*  Prefix for all counter paths */
							if (request.subscribe().has_prefix()) {
								Path* prefix = notification->mutable_prefix();
								prefix->set_target(request.subscribe().prefix().target());
							}

							// TODO : Notification.alias

							/* Embedded Update message inside Notification message */
							RepeatedPtrField<Update>* updateL = notification->mutable_update();
							Update *update = updateL->Add();
							/* If a directory path has been provided in the request, we must
							 * get all the leaves of the file tree. */
							Path *path = update->mutable_path();
							PathElem *pathElem = path->add_elem();
							pathElem->set_name("path_elem_name");
							TypedValue* val = update->mutable_val();
							val->set_string_val("Test message");
							update->set_duplicates(0);

							// TODO: Notification.delete

							// Notification.atomic
							notification->set_atomic(false);

							// Send first message: notification message
							std::cout << response.DebugString() << std::endl;
							stream->Write(response);

							// Send second message: sync message
							response.clear_update();
							response.set_sync_response(true);
							stream->Write(response);

							break;
						}
					case SubscriptionList_Mode_ONCE:
						{
							/* TODO: Same as above but no need for a thread to handle it */
							std::cout << "Received a ONCE subscription req" << std::endl;
							return Status(StatusCode::UNIMPLEMENTED,
														grpc::string("ONCE mode not implemented yet"));
							break;
						}
					case SubscriptionList_Mode_POLL:
						{
							std::cout << "Received a POLL subscription req" << std::endl;
							return Status(StatusCode::UNIMPLEMENTED,
														grpc::string("POLL mode not available"));
							break;
						}
					default:
						return Status(StatusCode::UNIMPLEMENTED,
													grpc::string("Mode not available"));
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
