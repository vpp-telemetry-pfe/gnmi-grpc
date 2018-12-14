/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */
/*
 * This file defines helpful method to manage encoding of the notification
 * message.
 */

#include <string>
#include <iostream>

#include "gnmi_encode.h"

using namespace gnmi;
using namespace std;
using namespace chrono;
using google::protobuf::RepeatedPtrField;

/**
 * split - split string in substrings according to delimitor.
 * @param str the string to parse.
 * @param delim the dilimitation character.
 */
vector<string> split( const string &str, const char &delim )
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
void UnixtoGnmiPath(string unixp, Path* path)
{
  vector<string> entries = split (unixp, '/');

  for (auto const& entry : entries) {
    PathElem *pathElem = path->add_elem();
    pathElem->set_name(entry);
    cout << entry << endl;
  }
}

string GnmiToUnixPath(Path *path)
{
  return "todo";
}

/**
 * BuildNotification - build a Notification message to answer a SubscribeRequest.
 * @param request the SubscribeRequest to answer to.
 * @param response the SubscribeResponse that is constructed by this function.
 */
void BuildNotification(
    const SubscribeRequest& request, SubscribeResponse& response)
{
  Notification *notification = response.mutable_update();
  RepeatedPtrField<Update>* updateList = notification->mutable_update();
  milliseconds ts;

  /* Get time since epoch in milliseconds */
  ts = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
  notification->set_timestamp(ts.count());

  /* Notification message prefix based on SubscriptionList prefix */
  if (request.subscribe().has_prefix()) {
    Path* prefix = notification->mutable_prefix();
    prefix->set_target(request.subscribe().prefix().target());
    // set name of measurement
    prefix->mutable_elem()->Add()->set_name("measurement1");
  }

  /* TODO : Path aliases defined by clients to access a long path.
   * We need to provide global variable with the list of aliases which can be
   * used only if use_aliases boolean is true in SubscriptionList. */
  if (request.subscribe().use_aliases())
    std::cerr << "Unsupported usage of aliases" << std::endl;

  /* TODO check if only updates should be sent
   * Require to implement a caching system to access last data sent. */
  if (request.subscribe().updates_only())
    std::cerr << "Unsupported usage of Updates, every paths will be sent"
              << std::endl;

  /* Fill Update RepeatedPtrField in Notification message
   * Update field contains only data elements that have changed values. */
  for (int i=0; i<request.subscribe().subscription_size(); i++) {
    Subscription sub = request.subscribe().subscription(i);
    RepeatedPtrField<Update>* updateList =
      notification->mutable_update();
    Update* update = updateList->Add();
    Path* path = update->mutable_path();
    TypedValue* val = update->mutable_val();

    /* TODO: Fetch the value from the stat_api instead of hardcoding a fake one
     * If succeeded Copy Request path into response path. */
    /* val can be:
     * - an unsigned integer on 64 bits: for SCALAR and ERROR
     * - a string:
     * - A Scalar Array:
     * - byte sequence value:
     * - a json encoded text:
     */
    val->set_string_val("Test message number " + to_string(i));
    path->CopyFrom(sub.path());

    update->set_duplicates(0);
  }

  notification->set_atomic(false);
}

/**
 * For testing purposes only
 */
//int main (int argc, char **argv)
//{
//  Path path;
//
//  UnixtoGnmiPath("/usr/local/bin", &path);
//
//  return 0;
//}
