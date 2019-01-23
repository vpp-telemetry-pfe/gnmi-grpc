/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */
#include <grpc/grpc.h>
#include "../proto/gnmi.grpc.pb.h"
#include "gnmi_stat.h"

#include <thread>

using namespace grpc;
using namespace gnmi;

class RequestHandler {
  public:
    Status handleSubscribeRequest(ServerContext* context,
      ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);

  private:
    Status handlePoll(ServerContext* context, SubscribeRequest request,
        ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);

    Status handleOnce(ServerContext* context, SubscribeRequest request,
      ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);

    Status handleStream(ServerContext* context, SubscribeRequest request,
      ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);

    void BuildNotification(const SubscriptionList& request,
                           SubscribeResponse& response);

  private:
    StatConnector statc;
};
