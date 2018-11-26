/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 smarttab: */
/*
 * This file defines helpful method to manage encoding of the notification
 * message.
 */

#include <string>
#include <iostream>

#include "gnmi_encode.h"

/* split - split string in substrings according to delimitor */
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

/* UnixtoGnmiPath - Convert a Unix Path to a GNMI Path
 * @param unixp Unix path.
 * @param path Pointer to GNMI path.
 */
void UnixtoGnmiPath(string unixp, Path* path)
{
	vector<string> entries = split (unixp, '/');

	for (auto const& entry : entries) {
		PathElem *pathElem = path->add_elem();
		pathElem->set_name(entry);
		std::cout << entry << std::endl;
	}
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
