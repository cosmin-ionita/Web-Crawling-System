#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <ostream>

#define SIZE 100000

using namespace std;

class Message
{

	public:

		int type;
		int size;

		char payload[SIZE];

		void set_message(int type, int size, char payload[]);

		char* serialize_message();
		void deserialize_message(char* message);

		void print_message();

};


#endif