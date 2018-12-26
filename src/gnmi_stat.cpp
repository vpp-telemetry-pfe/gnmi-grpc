// vim: noai:ts=2:sw=2:tw=2:expandtab

#include <vom/stat_client.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

typedef unsigned int u32;
typedef unsigned char u8;

using VOM::stat_client;
using namespace std;

/* DisplayPatterns - Print counter values of a set of counter path.
 * @return -1=patterns not found ; 0=no problem faced
 */
int DisplayPatterns(shared_ptr<stat_client> stat)
{
  /* Fill in VPP vector of stat indexes for metrics to collect */
  stat->ls();

  /* vector<stat_data_t>, it is possible to have an empty vector in res */
  stat_client::stat_data_vec_t res = stat->dump();

  for (auto counter = res.begin(); counter != res.end(); counter++) {
    switch (counter->type()) {
      case STAT_DIR_TYPE_COUNTER_VECTOR_SIMPLE:
        {
        cout << counter->name() << endl;
        uint64_t **matrix = counter->get_stat_segment_simple_counter_data();
        for (int k = 0; k < stat->vec_len(matrix); k++)
          for (int j = 0; j < stat->vec_len(matrix[k]); j++)
            cout << "thread, iface" << k << "," << j << ":"
                 << matrix[k][j] << endl;
        break;
        }
      case STAT_DIR_TYPE_COUNTER_VECTOR_COMBINED:
        {
        cout << counter->name() << endl;
        VOM::counter_t **matrix = counter->get_stat_segment_combined_counter_data();
        for (int k = 0; k < stat->vec_len(matrix); k++)
          for (int j = 0; j < stat->vec_len(matrix[k]); j++) {
            cout << "thread,iface" << k << "," << j << ":"
                 << matrix[k][j].packets << " "
                 << matrix[k][j].bytes << endl;
          }
        break;
        }
      case STAT_DIR_TYPE_ERROR_INDEX:
        cout << counter->name() << " "
             << counter->get_stat_segment_error_data() << endl;
        break;
      case STAT_DIR_TYPE_SCALAR_INDEX:
        cout << counter->name() << " "
             << counter->get_stat_segment_scalar_data() << endl;
        break;
      default:
        cerr << "Unknown value" << endl;
    }
  }

  return 0;
}

//For testing purpose only
int main (int argc, char **argv)
{
  vector<string> metrics{"/if", "/err", "/sys", "/err/udp6-input/"};
  shared_ptr<stat_client> stat(new stat_client(metrics));
  int rc;

  /* connect to STAT_SEMGENT_SOCKET_FILE */
  rc = stat->connect();
  if (rc < 0) {
    cerr << "can not connect to VPP STAT unix socket" << endl;
    exit(1);
  }  else {
    cout << "Connected to STAT socket" << endl;
  }

  DisplayPatterns(stat);

  stat->disconnect();
  cout << "Disconnect STAT socket" << endl;

  return 0;
}
