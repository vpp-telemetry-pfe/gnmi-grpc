#include "../proto/gnmi.grpc.pb.h"

using namespace gnmi;

void BuildNotification(
    const SubscriptionList& request, SubscribeResponse& response);
