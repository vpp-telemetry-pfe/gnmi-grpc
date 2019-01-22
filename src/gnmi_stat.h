/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

typedef unsigned int u32;
typedef unsigned char u8;

#include "../proto/gnmi.grpc.pb.h"

using google::protobuf::RepeatedPtrField;
using gnmi::Update;

class StatConnector
{
  public:
    StatConnector();
    ~StatConnector();

    void FillCounters(RepeatedPtrField<Update> *list, std::string metric);
};
