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

/**
 * Handles SubscribeRequest messages with STREAM subscription mode by
 * periodically sending updates to the client.
 */
Status handleStream(
    ServerContext* context, SubscribeRequest request,
    ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream)
{
  // Sends a first Notification message that updates all Subcriptions
  SubscribeResponse response;
  BuildNotification(request, response);
  cout << response.DebugString() << endl;
  stream->Write(response);
  response.Clear();

  // Sends a SYNC message that indicates that initial synchronization
  // has completed, i.e. each Subscription has been updated once
  response.set_sync_response(true);
  cout << response.DebugString() << endl;
  stream->Write(response);
  response.Clear();

  vector<pair<Subscription, time_point<system_clock>>> chronomap;
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

  /* Periodically updates paths that require SAMPLE updates
   * Note : There is only one Path per Subscription, but repeated
   * Subscriptions in a SubscriptionList, each Subscription can
   * have its own sample interval */
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
      response.Clear();
    }

    // Caps the loop at 5 iterations per second
    auto loopTime = high_resolution_clock::now() - start;
    this_thread::sleep_for(milliseconds(200) - loopTime);
  }

  return Status::OK;
}

/**
 * Handles SubscribeRequest messages with ONCE subscription mode by updating
 * all the Subscriptions once, sending a SYNC message, then closing the RPC.
 */
Status handleOnce(
    ServerContext* context, SubscribeRequest request,
    ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream)
{
  // Sends a Notification message that updates all Subcriptions once
  SubscribeResponse response;
  BuildNotification(request, response);
  cout << response.DebugString() << endl;
  stream->Write(response);
  response.Clear();

  // Sends a message that indicates that initial synchronization
  // has completed, i.e. each Subscription has been updated once
  response.set_sync_response(true);
  cout << response.DebugString() << endl;
  stream->Write(response);
  response.Clear();

  context->TryCancel();
  return Status::OK;
}

/**
 * Handles SubscribeRequest messages with POLL subscription mode by updating
 * all the Subscriptions each time a Poll request in received.
 */
Status handlePoll(
    ServerContext* context, SubscribeRequest request,
    ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream)
{
  SubscribeRequest subscription = request;
  while (stream->Read(&request)) {
    switch (request.request_case()) {
      case request.kPoll:
        {
          // Sends a Notification message that updates all Subcriptions once
          SubscribeResponse response;
          BuildNotification(subscription, response);
          cout << response.DebugString() << endl;
          stream->Write(response);
          response.Clear();
          break;
        }
      case request.kAliases:
        return Status(StatusCode::UNIMPLEMENTED, grpc::string(
              "Aliases not implemented yet"));
      case request.kSubscribe:
        return Status(StatusCode::INVALID_ARGUMENT, grpc::string(
              "A SubscriptionList has already been received for this RPC"));
      default:
        return Status(StatusCode::INVALID_ARGUMENT, grpc::string(
              "Unknown content for SubscribeRequest message"));
    }
  }
  return Status::OK;
}

/**
  * Handles the first SubscribeRequest message.
  * If it does not have the "subscribe" field set, the RPC MUST be cancelled.
  * Ref: 3.5.1.1
  */
Status handleSubscribeRequest(ServerContext* context,
    ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream)
{
  SubscribeRequest request;
  stream->Read(&request);
  if (!request.has_subscribe()) {
    context->TryCancel();
    return Status(StatusCode::INVALID_ARGUMENT, grpc::string(
          "SubscribeRequest needs non-empty SubscriptionList"));
  }

  switch (request.subscribe().mode()) {
    case SubscriptionList_Mode_STREAM:
      return handleStream(context, request, stream);
    case SubscriptionList_Mode_ONCE:
      return handleOnce(context, request, stream);
    case SubscriptionList_Mode_POLL:
      return handlePoll(context, request, stream);
    default:
      return Status(StatusCode::UNKNOWN,
          grpc::string("Unkown subscription mode"));
  }
  return Status::OK;
}
