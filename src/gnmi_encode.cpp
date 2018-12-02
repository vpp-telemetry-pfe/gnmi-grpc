/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 smarttab: */
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

/**
 * BuildNotification - build a Notification message to answer a SubscribeRequest.
 * @param request the SubscribeRequest to answer to.
 * @param response the SubscribeResponse that is constructed by this function.
 */
void BuildNotification(
    const SubscribeRequest& request, SubscribeResponse& response)
{
  Notification *notification = response.mutable_update();

  milliseconds ts;
  ts = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
  notification->set_timestamp(ts.count());

  // Notification.prefix
  if (request.subscribe().has_prefix()) {
    Path* prefix = notification->mutable_prefix();
    prefix->set_target(request.subscribe().prefix().target());
  }

  // TODO : Notification.alias

  // repeated Notification.update
  for (int i=0; i<request.subscribe().subscription_size(); i++) {
    Subscription sub = request.subscribe().subscription(i);
    RepeatedPtrField<Update>* updateList = 
      notification->mutable_update();
    Update* update = updateList->Add();

    Path* path = update->mutable_path();
    path->CopyFrom(sub.path());

    // TODO: Fetch the value from the stat_api instead of hardcoding a fake one
    TypedValue* val = update->mutable_val();
    val->set_string_val("Test message number " + to_string(i));

    update->set_duplicates(0);
  }

  // TODO: Notification.delete

  // Notification.atomic
  notification->set_atomic(false);
}

/**
 * For testing purposes only
 */
//int main (int argc, char **argv)
//{
//	Path path;
//
//	UnixtoGnmiPath("/usr/local/bin", &path);
//
//	return 0;
//}
