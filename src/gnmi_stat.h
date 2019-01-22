/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

typedef unsigned int u32;
typedef unsigned char u8;

#include "../proto/gnmi.grpc.pb.h"

class StatConnector
{
  public:
    StatConnector();
    ~StatConnector();

    u8 ** CreatePatterns(std::string metric);
    void FreePatterns(u8 **patterns);
    int DisplayPatterns(u8 **patterns);
    int FillCounter(gnmi::TypedValue* val, u8 **patterns);
};
