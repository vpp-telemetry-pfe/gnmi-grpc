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

#include "gnmi_stat.h"

using namespace std;
using namespace gnmi;

/**
 * split - split string in substrings according to delimitor.
 * @param str the string to parse.
 * @param delim the dilimitation character.
 */
vector<string> split(const string &str, const char &delim)
{
  typedef string::const_iterator iter;
  iter beg = str.begin();
  vector<string> tokens;

  while(beg != str.end()) {
    iter temp = find(beg, str.end(), delim);
    if(beg != str.end() && !string(beg,temp).empty())
      tokens.push_back(string(beg, temp));
    beg = temp;
    while ((beg != str.end()) && (*beg == delim))
      beg++;
  }

  return tokens;
}

/**
 * UnixtoGnmiPath - Convert a Unix Path to a GNMI Path.
 * @param unixp Unix path.
 * @param path Pointer to GNMI path.
 */
void UnixToGnmiPath(string unixp, Path* path)
{
  vector<string> entries = split (unixp, '/');

  for (auto const& entry : entries) {
    PathElem *pathElem = path->add_elem();
    pathElem->set_name(entry);
  }
}

/* createPatterns - Create a VPP vector containing set of paths as parameter for
 * stat_segment_ls.
 * @param metric UNIX paths to shared memory stats counters.
 * @return VPP vector containing UNIX paths or NULL in case of failure.
 */
u8 ** createPatterns(string metric)
{
  u8 **patterns = 0;

  patterns = stat_segment_string_vector(patterns, metric.c_str());
  if (!patterns)
    exit(-ENOMEM);

  return patterns;
}

/* freePatterns - Free a VPP vector created with CreatePatterns.
 * @param patterns VPP vector containing UNIX path of stats counters.
 */
void freePatterns(u8 **patterns)
{
  stat_segment_vec_free(patterns);
}

/* addIntCounter - Add a new Update in Notification answer with uint64 value
 * @param list Update List of Notification answer
 * @param path Unix Path of the counter
 * @param value counter value on 64 bits
 */
static inline void
addIntCounter(RepeatedPtrField<Update> *list, string path, uint64_t value)
{
    Update* update = list->Add();

    UnixToGnmiPath(path, update->mutable_path());
    update->mutable_val()->set_int_val(value);
    update->set_duplicates(0);
}

/** FillCounters - Fill val with counter value collected with STAT API
 * @param val counter value answered to gNMI client
 * @param patterns VPP vector containing UNIX path of stats counter.
 */
void StatConnector::FillCounters(RepeatedPtrField<Update> *list, string metric)
{
  stat_segment_data_t *r;
  u8 ** patterns = createPatterns(metric);
  u32 *stats = 0;

  do {
    stats = stat_segment_ls(patterns);
    if (!stats) {
      cerr << "No pattern was found" << endl;
      return;
    }

    r = stat_segment_dump(stats);
  } while (r == 0); /* Memory layout has changed */

  // Iterate over all subdirectories of requested path
  for (int i = 0; i < stat_segment_vec_len(r); i++) {
    switch (r[i].type) {
      case STAT_DIR_TYPE_COUNTER_VECTOR_SIMPLE:
        {
          int k = 0, j = 0;
          for (; k < stat_segment_vec_len(r[i].simple_counter_vec); k++)
            for (; j < stat_segment_vec_len(r[i].simple_counter_vec[k]); j++) {
              //path = counter + ifacename + thread num
              string path (r[i].name);
              path += '/' + VapiConnector::ifMap[j] + "/T" + to_string(k);
              addIntCounter(list, path, r[i].simple_counter_vec[k][j]);
            }
          break;
        }
      case STAT_DIR_TYPE_COUNTER_VECTOR_COMBINED:
        {
          int k = 0, j = 0;
          for (; k < stat_segment_vec_len(r[i].combined_counter_vec); k++)
            for (; j < stat_segment_vec_len(r[i].combined_counter_vec[k]); j++)
            {
              //path = counter + ifacename + thread num
              string path (r[i].name);
              path += '/' + VapiConnector::ifMap[j] + "/T" + to_string(k);
              addIntCounter(list, path + "/packets",
                  r[i].combined_counter_vec[k][j].packets);
              addIntCounter(list, path + "/bytes",
                  r[i].combined_counter_vec[k][j].bytes);
            }
          break;
        }
      case STAT_DIR_TYPE_ERROR_INDEX:
        addIntCounter(list, r[i].name, r[i].error_value);
        break;
      case STAT_DIR_TYPE_SCALAR_INDEX:
        addIntCounter(list, r[i].name, r[i].scalar_value);
        break;
      default:
        cerr << "Unknown value" << endl;
    }
  }
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

/* Connect - Connect to VPP to use VPP API */
VapiConnector::VapiConnector() {
  string app_name = "gnmi_server";
  char *api_prefix = nullptr;
  const int max_req = 32; /* max outstanding requests */
  const int response_queue_size = 32;
  vapi_error_e rv;

  rv = con.connect(app_name.c_str(), api_prefix, max_req, response_queue_size);
  if (rv != VAPI_OK) {
    cerr << "Error connecting to VPP API" << endl;
    exit(1);
  }
}

/* Disconnect - Disconnect from VPP API */
VapiConnector::~VapiConnector() {
  con.disconnect();
}

std::map <u32, std::string> VapiConnector::ifMap;

/* GetInterfaceDetails - Perform a dump information to fill map between
 * interfaces index and interfaces name.
 */
void VapiConnector::GetInterfaceDetails()
{
  vapi_error_e rv;

  //Dump: requ=vapi_msg_sw_interface_dump; resp=vapi_msg_sw_interface_details
  vapi::Sw_interface_dump req(con);

  rv = req.execute(); //send request
  if (rv != VAPI_OK)
    cerr << "request error" << endl;

  con.wait_for_response(req);
  for (auto& ifMsg : req.get_result_set()) {
    u32 index = ifMsg.get_payload().sw_if_index;
    string name ((char *)ifMsg.get_payload().interface_name);
    //Change '/' in '_' not to mistake with path delimiter
    std::replace(name.begin(), name.end(), '/', '_');
    ifMap.insert(pair<u32, string>(index, name));
  }
  needUpdate = false;
}

/* Callback for Sw_interface_event executed when event is received */
vapi_error_e VapiConnector::notify(if_event& ev)
{
  ev.get_result_set().free_all_responses(); //delete all events
  needUpdate = true; // after dispatch ends, it will launch dump
  return (VAPI_OK);
}

/* RegisterIfaceEvent - Ask for interface events using Want_interface_event
 * messages sending vapi_msg_want_interface_events msg and receiving
 * vapi_msg_want_interface_events_reply. Then, thread loop collect events.
 * Interface events are sent for interface creation but not deletion.
 */
void VapiConnector::RegisterIfaceEvent() {
  vapi_error_e rv;

  /* Register for interface events */
  vapi::Want_interface_events req(con);
  // Enable events and fill PID request field
  req.get_request().get_payload().pid = getpid();
  req.get_request().get_payload().enable_disable = 1;

  rv = req.execute(); // send request
  if (rv != VAPI_OK) {
    cerr << "request error" << endl;
    exit(rv);
  }

  con.wait_for_response(req);

  if (req.get_response().get_payload().retval != 0) {
    cerr << "Failed to connect to VPP API" << endl;
    exit(1);
  }

  /* Thread Loop collecting in charge of updating ifMap */
  Functor functor(this);
  if_event ev(con, functor);
  GetInterfaceDetails(); //Get Map at the beginning
  while (1) {
    con.dispatch(ev);
    if (needUpdate)
      GetInterfaceDetails();
  }
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
