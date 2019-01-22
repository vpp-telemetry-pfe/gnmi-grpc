// vim: softtabstop=2 shiftwidth=2 tabstop=2 expandtab:

#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

extern "C" {
#include <vpp-api/client/stat_client.h>
}
#include <vapi/interface.api.vapi.hpp>
#include <vapi/vapi.hpp>

#include "gnmi_stat.h"

using namespace std;

/* CreatePatterns - Create a VPP vector containing set of paths as parameter for
 * stat_segment_ls.
 * @param metric UNIX paths to shared memory stats counters.
 * @return VPP vector containing UNIX paths or NULL in case of failure.
 */
u8 ** StatConnector::CreatePatterns(string metric)
{
  u8 **patterns = 0;

  patterns = stat_segment_string_vector(patterns, metric.c_str());
  if (!patterns) {
    exit(-ENOMEM);
  }

  return patterns;
}

/* FreePatterns - Free a VPP vector created with CreatePatterns.
 * @param patterns VPP vector containing UNIX path of stats counters.
 */
void StatConnector::FreePatterns(u8 **patterns)
{
  stat_segment_vec_free(patterns);
}

/** FillCounter - Fill val with counter value collected with STAT API
 * @param val counter value answered to gNMI client
 * @param patterns VPP vector containing UNIX path of stats counter.
 */
int StatConnector::FillCounter(gnmi::TypedValue *val, u8 **patterns)
{
  stat_segment_data_t *res;
  static u32 *stats = 0;

  do {
    stats = stat_segment_ls(patterns);
    if (!stats) {
      cerr << "No pattern was found" << endl;
      return -1;
    }

    res = stat_segment_dump(stats);
  } while (res == 0); /* Memory layout has changed */

  for (int i = 0; i < stat_segment_vec_len(res); i++) {
    switch (res[i].type) {
      case STAT_DIR_TYPE_COUNTER_VECTOR_SIMPLE:
        {
          string tmp = res[i].name;
          tmp.append(" ");
          for (int k = 0;
              k < stat_segment_vec_len(res[i].simple_counter_vec);
              k++)
            for (int j = 0;
                j < stat_segment_vec_len(res[i].simple_counter_vec[k]);
                j++)
              tmp += res[i].simple_counter_vec[k][j];
          val->set_string_val(tmp);
          break;
        }
      case STAT_DIR_TYPE_COUNTER_VECTOR_COMBINED:
        {
          string tmp = res[i].name;
          tmp.append(" ");
          for (int k = 0;
              k < stat_segment_vec_len(res[i].combined_counter_vec);
              k++)
            for (int j = 0;
                j < stat_segment_vec_len(res[i].combined_counter_vec[k]);
                j++) {
              tmp += res[i].combined_counter_vec[k][j].packets;
              tmp.append(" ");
              tmp+=res[i].combined_counter_vec[k][j].bytes;
            }
          val->set_string_val(tmp);
          break;
        }
      case STAT_DIR_TYPE_ERROR_INDEX:
        val->set_string_val(res[i].name + res[i].error_value);
        break;
      case STAT_DIR_TYPE_SCALAR_INDEX:
        val->set_string_val(res[i].name + std::to_string(res[i].scalar_value));
        break;
      default:
        cerr << "Unknown value" << endl;
    }
  }
  return 0;
}

/** Connect to VPP STAT API */
StatConnector::StatConnector()
{
  char socket_name[] = STAT_SEGMENT_SOCKET_FILE;
  int rc;

  rc = stat_segment_connect(socket_name);
  if (rc < 0) {
    cerr << "can not connect to VPP STAT unix socket" << endl;
    exit(1);
  }
}

/** Disconnect from VPP STAT API */
StatConnector::~StatConnector()
{
  stat_segment_disconnect();
  cout << "Disconnect STAT socket" << endl;
}

//////////////////////////////////////////////////////////////
// This part is about VPP API to handle conversion of indexes to interface name

/* Connect - Connect to VPP to use VPP API */
void Connect(vapi::Connection& con) {
  string app_name = "gnmi_server";
  char *api_prefix = nullptr;
  const int max_req = 32; /* max outstanding requests */
  const int response_queue_size = 32;
  vapi_error_e rv;

  rv = con.connect(app_name.c_str(), api_prefix, max_req, response_queue_size);
  if (rv != VAPI_OK) {
    cerr << "conn error" << endl;
    exit(1);
  }
}

/* Disconnect - Disconnect from VPP API */
void Disconnect(vapi::Connection& con) {
  con.disconnect();
}

/* GetInterfaceDetails - RPC client interacting directly with VPP binary API to
 * collect interfaces names to send in telemetry messages. */
void GetInterfaceDetails(vapi::Connection& con) {
  vapi_error_e rv;

  /* Create a Dump object with vapi_msg_sw_interface_dump request and
   * vapi_msg_sw_interface_details response */
  vapi::Sw_interface_dump req(con);

  /* send the request */
  rv = req.execute();
  if (rv != VAPI_OK)
    cerr << "request error" << endl;

  con.wait_for_response(req);
  for (auto it = req.get_result_set().begin(); it != req.get_result_set().end();
      it++) {
    cout << "sw_if_index: " << it->get_payload().sw_if_index << "\n"
      << "interface_name: " << it->get_payload().interface_name << "\n"
      << "MAC address: " << it->get_payload().l2_address
      << endl;
  }
}

/* RegisterIfaceEvent - Ask for interface events using Want_interface_event
 * messages. Interface events are sent when an interface is created but not
 * when deleted. */
void RegisterIfaceEvent(vapi::Connection& con) {
  vapi_error_e rv;

  /* Create an object with vapi_msg_want_interface_events request and
   * vapi_msg_want_interface_events_reply */
  vapi::Want_interface_events req(con);

  /* Set Request fields to enable interface events */
  req.get_request().get_payload().pid = getpid();
  req.get_request().get_payload().enable_disable = 1;
  cout << "pid: " << req.get_request().get_payload().pid << "\n"
    << "enable/disable: " << req.get_request().get_payload().enable_disable
    << endl;

  /* Send the request */
  rv = req.execute();
  if (rv != VAPI_OK)
    cerr << "request error" << endl;

  con.wait_for_response(req);

  cout << "retvalue: " << req.get_response().get_payload().retval << endl;
}

typedef vapi::Event_registration<vapi::Sw_interface_event> if_event;

vapi_error_e notify(if_event& ev) {
  cout << "Reeeeeeeeeeeeeeeeeeceeeeeeeeeeeeeeeived" << endl;

  for (auto it = ev.get_result_set().begin(); it != ev.get_result_set().end();
      it++) {
    cout << "id: "            << it->get_payload()._vl_msg_id << "\n"
      << "sw_if_index: "   << it->get_payload().sw_if_index << "\n"
      << "admin up/down: " << it->get_payload().admin_up_down << "\n"
      << "link up/down: "  << it->get_payload().link_up_down << "\n"
      << "deleted: "       << it->get_payload().deleted << "\n"
      << endl;
  }

  return (VAPI_OK);
}

/* DisplayIfaceEvent - This must run inside a thread */
void DisplayIfaceEvent(vapi::Connection &con) {
  if_event ev(con, notify);
  con.dispatch(ev);
}

//For testing purpose only
/*
   int main (int argc, char **argv)
   {
   u8 **patterns = 0;
   string metric{"/if"};
   char socket_name[] = STAT_SEGMENT_SOCKET_FILE;
   int rc;

   rc = stat_segment_connect(socket_name);
   if (rc < 0) {
   cerr << "can not connect to VPP STAT unix socket" << endl;
   exit(1);
   }
   cout << "Connected to STAT socket" << endl;

   patterns = CreatePatterns(metric);
   if (!patterns)
   return -ENOMEM;
   DisplayPatterns(patterns);
   FreePatterns(patterns);

// VPP API functions
vapi::Connection con;
Connect(con);
GetInterfaceDetails(con);
RegisterIfaceEvent(con);
DisplayIfaceEvent(con);
cout << "start sleeping" << endl;
Disconnect(con);

stat_segment_disconnect();
cout << "Disconnect STAT socket" << endl;

return 0;
} */
