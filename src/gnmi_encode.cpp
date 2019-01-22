/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */
/*
 * This file defines helpful method to manage encoding of the notification
 * message.
 */

#include <string>
#include <iostream>

#include "gnmi_encode.h"
#include "gnmi_stat.h"

using namespace gnmi;
using namespace std;
using namespace chrono;
using google::protobuf::RepeatedPtrField;

/* GnmiToUnixPath - Convert a GNMI Path to UNIX Path
 * @param path the Gnmi Path
 */
string GnmiToUnixPath(Path path)
{
  string uxpath;

  for (int i=0; i < path.elem_size(); i++) {
    uxpath += "/";
    uxpath += path.elem(i).name();
  }

  return uxpath;
}

/**
 * BuildNotification - build a Notification message to answer a SubscribeRequest.
 * @param request the SubscriptionList from SubscribeRequest to answer to.
 * @param response the SubscribeResponse that is constructed by this function.
 */
void BuildNotification(
    const SubscriptionList& request, SubscribeResponse& response)
{
  Notification *notification = response.mutable_update();
  RepeatedPtrField<Update>* updateList = notification->mutable_update();
  milliseconds ts;

  /* Get time since epoch in milliseconds */
  ts = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
  notification->set_timestamp(ts.count());

  /* Notification message prefix based on SubscriptionList prefix */
  if (request.has_prefix()) {
    Path* prefix = notification->mutable_prefix();
    prefix->set_target(request.prefix().target());
    // set name of measurement
    prefix->mutable_elem()->Add()->set_name("measurement1");
  }

  // Defined refer to a long Path by a shorter one: alias
  if (request.use_aliases())
    cerr << "Unsupported usage of aliases" << endl;

  /* TODO check if only updates should be sent
   * Require to implement a caching system to access last data sent. */
  if (request.updates_only())
    cerr << "Unsupported usage of Updates, every paths will be sent"  << endl;

  /* Fill Update RepeatedPtrField in Notification message
   * Update field contains only data elements that have changed values. */
  for (int i = 0; i < request.subscription_size(); i++) {
    Subscription sub = request.subscription(i);

    // Fetch all found counters value for a requested path
    StatConnector stat = StatConnector();
    cout << "Requested: " + GnmiToUnixPath(sub.path()) << endl;
    stat.FillCounters(updateList, GnmiToUnixPath(sub.path()));
  }

  notification->set_atomic(false);
}
