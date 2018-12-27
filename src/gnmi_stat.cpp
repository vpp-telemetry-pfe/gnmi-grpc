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

typedef unsigned int u32;
typedef unsigned char u8;

using namespace std;

/* CreatePatterns - Create a VPP vector containing set of paths as parameter for
 * stat_segment_ls.
 * @param metrics Vector of UNIX paths to shared memory stats counters.
 * @return VPP vector containing UNIX paths or NULL in case of failure.
 */
u8 ** CreatePatterns(vector<string> metrics)
{
  char * pattern = 0;
  u8 **patterns = 0;

  for (vector<string>::iterator it = metrics.begin(); it != metrics.end();
      it++) {
    pattern = strdup(it[0].c_str());
    if (!pattern)
      return NULL;

    patterns = stat_segment_string_vector(patterns, pattern);
    free(pattern);
  }

  return patterns;
}

/* FreePatterns - Free a VPP vector created with CreatePatterns.
 * @param patterns VPP vector containing UNIX path of stats counters.
 */
void FreePatterns(u8 **patterns) {stat_segment_vec_free(patterns);}

/* DisplayPatterns - Print counter values of a set of counter path.
 * @param patterns VPP vector containing UNIX path of stats counter.
 * @return -1=patterns not found ; 0=no problem faced
 */
int DisplayPatterns(u8 **patterns)
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
        for (int k = 0;
             k < stat_segment_vec_len(res[i].simple_counter_vec);
             k++)
          for (int j = 0;
               j < stat_segment_vec_len(res[i].simple_counter_vec[k]);
               j++)
            cout << res[i].simple_counter_vec[k][j] << endl;
        break;
      case STAT_DIR_TYPE_COUNTER_VECTOR_COMBINED:
        cout << res[i].name << endl;
        for (int k = 0;
             k < stat_segment_vec_len(res[i].combined_counter_vec);
             k++)
          for (int j = 0;
               j < stat_segment_vec_len(res[i].combined_counter_vec[k]);
               j++) {
            cout << res[i].combined_counter_vec[k][j].packets
                 << res[i].combined_counter_vec[k][j].bytes << endl;
          }
        break;
      case STAT_DIR_TYPE_ERROR_INDEX:
        cout << res[i].name << " " << res[i].error_value << endl;
        break;
      case STAT_DIR_TYPE_SCALAR_INDEX:
        cout << res[i].name << " " << res[i].scalar_value << endl;
        break;
      default:
        cerr << "Unknown value" << endl;
    }
  }
  return 0;
}

/*
 * GetInterfaceDetails - RPC client interacting directly with VPP binary API.
 */
void GetInterfaceDetails() {
	vapi::Connection con;
	vapi_error_e rv;

	/* Connect to VPP */
	string app_name = "gnmi_server";
	char *api_prefix = nullptr;
	const int max_outstanding_requests = 32;
	const int response_queue_size = 32;

	rv = con.connect(app_name.c_str(), api_prefix, max_outstanding_requests,
                   response_queue_size);
	if (rv != VAPI_OK) {
		cerr << "conn error" << endl;
		return;
	}

	/* Create a RPC object */
	vapi::Sw_interface_dump req(con);

	/* send the request */
	rv = req.execute();

	con.wait_for_response(req);
	for (auto it = req.get_result_set().begin(); it != req.get_result_set().end();
      it++) {
		cout << "sw_if_index: " << it->get_payload().sw_if_index << "\n"
		     << "interface_name: " << it->get_payload().l2_address << "\n"
		     << "MAC address: " << it->get_payload().interface_name
		     << endl;
	}

	/* Disconnect */
	con.disconnect();
}

//For testing purpose only
int main (int argc, char **argv)
{
  u8 **patterns = 0;
  vector<string> metrics{"/if", "/err", "/sys", "/err/udp6-input/"};
  char socket_name[] = STAT_SEGMENT_SOCKET_FILE;
  int rc;

  rc = stat_segment_connect(socket_name);
  if (rc < 0) {
    cerr << "can not connect to VPP STAT unix socket" << endl;
    exit(1);
  }  else
    cout << "Connected to STAT socket" << endl;

  patterns = CreatePatterns(metrics);
  if (!patterns)
    return -ENOMEM;
  DisplayPatterns(patterns);
  FreePatterns(patterns);

  InterfaceDetails();

  stat_segment_disconnect();
  cout << "Disconnect STAT socket" << endl;

  return 0;
}
