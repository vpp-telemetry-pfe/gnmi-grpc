/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <iostream>
#include <memory>
#include <chrono>
#include <pthread.h>
#include <getopt.h>
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

void BuildNotification(
    const SubscribeRequest& request, SubscribeResponse& response)
{
  Notification *notification = response.mutable_update();
  milliseconds ts;

  /*  Get non-monotic timestamp since epoch in msecs when data is
    *  generated */
  ts = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
  notification->set_timestamp(ts.count());

  // Prefix for all counter paths
  if (request.subscribe().has_prefix()) {
    Path* prefix = notification->mutable_prefix();
    prefix->set_target(request.subscribe().prefix().target());
  }

  // TODO : Notification.alias

  // repeated Notification.update
  for (int i=0; i<request.subscribe().subscription_size(); i++) {
    Subscription sub = request.subscribe().subscription(i);
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
    TypedValue* val = update->mutable_val();
    val->set_string_val("Test message number " + std::to_string(i));
    update->set_duplicates(0);
  }

  // TODO: Notification.delete

  // Notification.atomic
  notification->set_atomic(false);
}

void* StreamRoutine(void* threadarg)
{
  std::cout << "Thread launched" << std::endl;
  SubscribeRequest* request = (SubscribeRequest*) threadarg;
  std::cout << request->DebugString() << std::endl;
  // TODO: Handle the different STREAM SubscriptionModes
  /*
  switch (sub.mode()) {
    case TARGET_DEFINED: { break; }
    case ON_CHANGE: { break; }
    case SAMPLE: { break; }
    default: { break; }
  }
  */
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

      /* TODO Retrieve Server Context metadata */
      std::multimap<grpc::string_ref, grpc::string_ref> metadata;
      metadata = context->client_metadata();

      for (std::multimap<grpc::string_ref, grpc::string_ref>::const_iterator iter = metadata.begin();
           iter != metadata.end();
           ++iter ) {
        cout << iter->first << '\t' << iter->second << '\n';
      }
      /* TODO: End Of TODO */

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

              BuildNotification(request, response);

              // Send first message: notification message
              std::cout << "Response message:" << std::endl;
              std::cout << response.DebugString() << std::endl;
              stream->Write(response);

              // Send second message: sync message
              response.clear_update();
              response.set_sync_response(true);
              std::cout << response.DebugString() << std::endl;
              stream->Write(response);

              // Launch dedicated thread to keep streaming updates
              pthread_t thread;
              if (pthread_create(
                    &thread, NULL, StreamRoutine, (void *) &request)) {
                std::cout << "Error launching thread" << std::endl;
              }
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

static void show_usage(std::string name)
{
  std::cerr << "Usage: " << name << " <option(s)>\n"
    << "Options:\n"
    << "\t-h,--help\tShow this help message\n"
    << "\t-u,--username USERNAME\tDefine connection username\n"
    << "\t-p,--password PASSWORD\tDefine connection password\n"
    << "\t-t,--tls \tUse TLS encryption communication"
    << std::endl;
}

int main (int argc, char* argv[]) {
  int c;
  int option_index = 0;
  extern char * optarg;

  static struct option long_options[] =
  {
    {"help", no_argument, 0, 'h'},
    {"username", required_argument, 0, 'u'},
    {"password", required_argument, 0, 'p'},
    {"tls", no_argument, 0, 't'},
    {0, 0, 0, 0}
  };

  /* "a::b:c:" means a is optional, b & c are mandatory */
  while ((c = getopt_long(argc, argv, "hp::u::t", long_options, &option_index)) != -1) {
    switch (c)
    {
      case 'h':
        {
          show_usage(argv[0]);
          exit(0);
          break;
        }
      case 'u':
        {
          if (optarg) {
            std::string username = string(optarg);
            std::cout << "username: " << username << std::endl;
          } else {
            std::cerr << "Please specify a string with username option\n"
              << "Ex: -uUSERNAME" << std::endl;
            exit(1);
          }
          break;
        }
      case 'p':
        {
          if (optarg) {
            std::string password = string(optarg);
            std::cout << "password: " << password << std::endl;
          } else {
            std::cerr << "Please specify a string with password option\n"
              << "Ex: -pPASSWORD" << std::endl;
            exit(1);
          }
          break;
        }
      case 't':
        std::cout << "tls option set" << std::endl;
      default:
        {
          show_usage(argv[0]);
          exit(0);
          break;
        }
    }
  }

  RunServer();
}
