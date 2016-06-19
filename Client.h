#ifndef CLIENT_H
#define CLIENT_H

#include <cstring>

class Client
{
	public:

		Client(char* IP, unsigned short port, int availability, int file_descriptor) 
		{
			strcpy(this->IP, IP);

			this->port = port;
			this->file_descriptor = file_descriptor;

			this->availability = availability;
		}

		char IP[13];
		unsigned short port;

		int availability, file_descriptor;

		bool operator>(const Client c) const;
};

struct ClientComparator
{
  bool operator()(const Client& n, const Client& p) const
  {
    return n.availability > p.availability;
  }
};


#endif