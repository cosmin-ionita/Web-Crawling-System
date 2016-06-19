#ifndef LINK_H
#define LINK_H

#include <string>
#include <cstring>
using namespace std;

class Link
{
	public:

		Link(int level, const char link[])
		{
			this->level = level;
			strcpy(this->link, link);
		}

		int level;
		char link[1000];

		char* get_transferable_link(bool recursive, bool everything);
};

#endif