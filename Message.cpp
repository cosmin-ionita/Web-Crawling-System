#include "Message.h"
#include <cstring>
#include <iostream>
#include <sstream>


void Message::set_message(int type, int size, char payload[])
{
	memset(this->payload, 0, SIZE);

	this->type = type;
	this->size = size;

	memcpy(this->payload, payload, size);
}

void Message::print_message()
{
	cout<<this->type<<" -> "<<this->size<<" -> "<<this->payload<<endl;
}

char* Message::serialize_message()
{
	char* result = new char[SIZE + 8];

	// In the first 4 bytes there is the message type
	// In the next 4 bytes there is the message dimension
	// In the last 100000 bytes there is the content of the message
	
	result[3] = ((this->type)>>24) & 0xFF;
	result[2] = ((this->type)>>16) & 0xFF;
	result[1] = ((this->type)>>8) & 0xFF;
	result[0] = (this->type) & 0xFF;

	result[7] = ((this->size)>>24) & 0xFF;
	result[6] = ((this->size)>>16) & 0xFF;
	result[5] = ((this->size)>>8) & 0xFF;
	result[4] = (this->size) & 0xFF;

	memcpy(result + 8, this->payload, SIZE);

	return result;
}

void Message::deserialize_message(char* message)
{
	char number[4];
	memset(number, 0, 4);
	memcpy(number, message, 4);

	this->type = *(int *)number;

	memset(number, 0, 4);
	memcpy(number, message + 4, 4);

	this->size = *(int *)number;

	memcpy(this->payload, message + 8, SIZE);
}
