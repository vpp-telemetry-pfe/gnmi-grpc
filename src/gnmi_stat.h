/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

typedef unsigned int u32;
typedef unsigned char u8;

#include <map>
#include <vapi/interface.api.vapi.hpp>
#include <vapi/vapi.hpp>
#include "../proto/gnmi.grpc.pb.h"

using google::protobuf::RepeatedPtrField;
using gnmi::Update;
using vapi::Connection;

class StatConnector
{
  public:
    StatConnector();
    ~StatConnector();

    void FillCounters(RepeatedPtrField<Update> *list, std::string metric);
};

//New type for interface events
typedef vapi::Event_registration<vapi::Sw_interface_event> if_event;

/* Connector to VPP API to handle conversion of indexes to interface name */
class VapiConnector {
  public:
    VapiConnector();
    ~VapiConnector();

    void GetInterfaceDetails();
    void RegisterIfaceEvent();
    void DisplayIfaceEvent();

  private:
    Connection con;
    std::map <u32, std::string> ifMap; //Map of sw_if_index, interface_name
};
