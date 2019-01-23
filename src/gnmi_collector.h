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

class StatConnector;
class VapiConnector;

class StatConnector
{
  public:
    StatConnector();
    ~StatConnector();

    void FillCounters(RepeatedPtrField<Update> *list, std::string metric);

  friend VapiConnector;
};

//New type for interface events
typedef vapi::Event_registration<vapi::Sw_interface_event> if_event;

/* Connector to VPP API to handle conversion of indexes to interface name */
class VapiConnector {
  public:
    VapiConnector();
    ~VapiConnector();

    void RegisterIfaceEvent();
    void GetInterfaceDetails();
    vapi_error_e notify(if_event& ev);

  private:
    Connection con;
    bool needUpdate = false;
    //Map of sw_if_index, interface_name
    static std::map <u32, std::string> ifMap;

  friend StatConnector;
};

/* Required to use notify as a non-static callback */
class Functor {
  public:
    Functor(VapiConnector* instance) : instance(instance) {}
    vapi_error_e operator()(if_event& ev)
    {
      return instance->notify(ev);
    }

  private:
    VapiConnector *instance;
};
