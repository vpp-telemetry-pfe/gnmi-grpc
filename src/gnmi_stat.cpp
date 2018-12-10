// vim: softtabstop=2 shiftwidth=2 tabstop=2 expandtab:

#include <vpp-api/client/stat_client.h>
#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

typedef unsigned int u32;
typedef unsigned char u8;

using namespace std;

u8 ** CreatePatterns(vector<string> metrics)
{
  char * pattern = 0;
  u8 **patterns = 0;

  for (std::vector<string>::iterator it = metrics.begin(); it != metrics.end();
       it++) {
    pattern = strdup(it[0].c_str());
    if (!pattern)
      return NULL;

    patterns = stat_segment_string_vector(patterns, pattern);
    free(pattern);
  }

  return patterns;
}

void FreePatterns(u8 **patterns) {stat_segment_vec_free(patterns);}

//For testing purpose only
int main (int argc, char **argv)
{
  stat_segment_data_t *res;
  static u32 *stats = 0;
  u8 **patterns = 0;
  //vector<string> metrics{"/if", "/err", "/sys", "/err/udp6-input/"};
  vector<string> metrics{"/err/udp6-input/"};
  char socket_name[] = "/var/run/vpp/stats.sock";
  int rc;

  rc = stat_segment_connect(socket_name);
  if (rc < 0)
    cerr << "can not connect to VPP STAT unix socket" << endl;
  else
    cout << "Connected to STAT socket" << endl;

  patterns = CreatePatterns(metrics);
  cout << "patterns vector length: " << stat_segment_vec_len(patterns) << endl;

  stats = stat_segment_ls(patterns);
  res = stat_segment_dump(stats);
  if (!res)
    std::cerr << "Memory layout has changed" << std::endl;

  for (int i = 0; i < stat_segment_vec_len(res); i++) {
    std::cout << res[i].name << std::endl;
    switch (res[i].type) {
      case STAT_DIR_TYPE_COUNTER_VECTOR_SIMPLE:
        for (int k = 0; k < stat_segment_vec_len(res[i].simple_counter_vec); k++)
          for (int j = 0; j < stat_segment_vec_len(res[i].simple_counter_vec[k]); j++)
            std::cout << "\t" << res[i].simple_counter_vec[k][j] << std::endl;
        break;
      case STAT_DIR_TYPE_COUNTER_VECTOR_COMBINED:
        break;
      case STAT_DIR_TYPE_ERROR_INDEX:
        std::cout << "\t" << res[i].error_value << std::endl;
        break;
      case STAT_DIR_TYPE_SCALAR_INDEX:
        std::cout << "\t" << res[i].scalar_value << std::endl;
        break;
      default:
        std::cerr << "Unknown value" << std::endl;
    }
  }

  FreePatterns(patterns);

  stat_segment_disconnect();
  cout << "Disconnect STAT socket" << endl;

  return 0;
}
