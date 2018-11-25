#include "../proto/gnmi.grpc.pb.h"

using namespace std;

using gnmi::Path;
using gnmi::PathElem;

void UnixtoGnmiPath(string unixp, Path* path);
