#include "Link.h"

#include <cstring>
#include <iostream>

char* Link::get_transferable_link(bool recursive, bool everything)
{
	char* result = new char[strlen(link) + 2];

	result[0] = level;
	result[1] = 0; 		// simple mode
	
	if(recursive && everything)
		result[1] = 3;
	else if(recursive)
		result[1] = 1;
	else if(everything)
		result[1] = 2;

	strcpy(result + 2, this->link);

	return result;
}