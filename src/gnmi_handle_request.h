#include <grpc/grpc.h>
#include "../proto/gnmi.grpc.pb.h"

using namespace grpc;
using namespace gnmi;

Status handleSubscribeRequest(ServerContext* context,
    ServerReaderWriter<SubscribeResponse, SubscribeRequest>* stream);
