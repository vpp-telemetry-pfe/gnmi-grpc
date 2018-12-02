#include "../proto/gnmi.grpc.pb.h"

using namespace std;
using namespace gnmi;

void UnixtoGnmiPath(string unixp, Path* path);
void BuildNotification(
    const SubscribeRequest& request, SubscribeResponse& response);
