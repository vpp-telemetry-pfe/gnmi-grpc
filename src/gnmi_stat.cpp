// vim: ts=2:sw=2:tw=2:expandtab

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <vom/stat_client.hpp>
#include <vom/interface.hpp>
#include <vom/om.hpp>

typedef unsigned int u32;
typedef unsigned char u8;

using VOM::stat_client;
using VOM::interface;
using namespace std;

/* DisplayCounters - Print counter values of a set of counter path. */
void DisplayCounters(shared_ptr<stat_client> stat)
{
  /* Fill in VPP vector of stat indexes for metrics to collect */
  stat->ls();

  /* Retrieve a vector of all counters matching indexes collected by ls */
  stat_client::stat_data_vec_t res = stat->dump();

  /* Display all counters */
  for (auto counter : res) {
    switch (counter.type()) {
      case STAT_DIR_TYPE_COUNTER_VECTOR_SIMPLE:
        {
        cout << counter.name() << "{" << endl;
        uint64_t **matrix = counter.get_stat_segment_simple_counter_data();
        for (int k = 0; k < stat->vec_len(matrix); k++)
          for (int j = 0; j < stat->vec_len(matrix[k]); j++) {
            cout << "\tinterface" << j << ":" << matrix[k][j] << endl;
          }
        cout << "}" << endl;
        break;
        }
      case STAT_DIR_TYPE_COUNTER_VECTOR_COMBINED:
        {
        cout << counter.name() << "{" << endl;
        VOM::counter_t **matrix =
          counter.get_stat_segment_combined_counter_data();
        for (int k = 0; k < stat->vec_len(matrix); k++)
          for (int j = 0; j < stat->vec_len(matrix[k]); j++) {
            cout << "\tinterface " << j << "{\n"
                 << "\t\tpackets:" << matrix[k][j].packets << "\n"
                 << "\t\tbytes:" << matrix[k][j].bytes << "\n"
                 << "\t}" << endl;
          }
          cout << "}" << endl;
        break;
        }
      case STAT_DIR_TYPE_ERROR_INDEX:
        cout << counter.name() << " "
             << counter.get_stat_segment_error_data() << endl;
        break;
      case STAT_DIR_TYPE_SCALAR_INDEX:
        cout << counter.name() << " "
             << counter.get_stat_segment_scalar_data() << endl;
        break;
      default:
        cerr << "Unknown value" << endl;
    }
  }
}

//For testing purpose only
int main (int argc, char **argv)
{
  vector<string> metrics{"/if", "/err", "/sys", "/err/udp6-input/"};
  shared_ptr<stat_client> stat(new stat_client(metrics));

  /* Initialize a VOM client and object model */
  VOM::HW::init(new VOM::HW::cmd_q());
  VOM::OM::init();

  /* Connect to VPP STAT shared memory */
  while (VOM::HW::connect() != true);

  /* Populate the OM database with VPP HW interfaces with key __DISCOVERED__.
   * It has to be event-based in order to be aware in case an interface is
   * removed or created. */
  VOM::OM::populate("__DISCOVERED__");
  for (auto itf = interface::cbegin(); itf != interface::cend(); itf++) {
    cout << "Name:" << itf->second.lock()->name() << "\n" //name of iface
         << "Handle:" << itf->second.lock()->handle() << "\n" //index in FIB
         << "Type:" << itf->second.lock()->type() << "\n" //type
         << "Key:" << itf->second.lock()->key() << endl; //key
  } //TODO populate once then use events

  DisplayCounters(stat);

  /* Remove __DISCOVERED__ entries added with populate */
  VOM::OM::remove("__DISCOVERED__");

  VOM::HW::disconnect();

  return 0;
}
