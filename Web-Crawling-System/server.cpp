#include <iostream>
#include <string>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <queue>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include "Client.h"
#include "Link.h"
#include "Message.h"

#define MAX_CLIENTS 10

using namespace std;

int _server_socket, _max_fd_range;
struct sockaddr_in _connection_data, _client_conn_data;

bool recursive = false, everything = false;

fd_set _read_fds, _tmp_fds;

queue<Client> online_clients;

vector<Link> links;

void is_valid(int connection_object, string entity) 
{
	if(connection_object < 0) 
		cout << "Failure detected at: " << entity <<"\n";
}

void init_connection_parameters(char* port)
{
	 _connection_data.sin_family = AF_INET;
	 _connection_data.sin_addr.s_addr = INADDR_ANY;
     _connection_data.sin_port = htons(atoi(port));
}

void initialize_server(char* port)
{
	init_connection_parameters(port);

	_server_socket = socket(AF_INET, SOCK_STREAM, 0);
	is_valid(_server_socket, "socket");

	int bind_result = bind(_server_socket, (struct sockaddr *) &_connection_data, sizeof(struct sockaddr));
	is_valid(bind_result, "bind");

	int listen_result = listen(_server_socket, MAX_CLIENTS);
	is_valid(listen_result, "listen");

	FD_ZERO(&_read_fds);
	FD_ZERO(&_tmp_fds);

	FD_SET(_server_socket, &_read_fds);
	FD_SET(0, &_read_fds);
	_max_fd_range = _server_socket;
}

void accept_handler() 
{
	unsigned int len = sizeof(_client_conn_data);

	int new_sock_fd = accept(_server_socket, (struct sockaddr *)&_client_conn_data, &len);
	is_valid(new_sock_fd, "accept");

	FD_SET(new_sock_fd, &_read_fds);

	if(new_sock_fd > _max_fd_range)
		_max_fd_range = new_sock_fd;

	cout<<"\nNew client from IP = "<<inet_ntoa(_connection_data.sin_addr)<<" and port = "<<_connection_data.sin_port<<"\n";

	online_clients.push(Client(inet_ntoa(_connection_data.sin_addr), _connection_data.sin_port, 0, new_sock_fd));
}

void print_links()
{
	for(vector<Link>::iterator it = links.begin(); it != links.end(); it++)
	{
		cout<<(*it).level<<" -> "<<(*it).link<<"\n";
	}	
}

void send_link(int client, Link link)
{
	Message message;

	message.set_message(link.level, strlen(link.link), link.link);

	char* result = message.serialize_message();

	send(client, result, SIZE + 8, 0);
}

bool exists_directory(char path[])
{
	DIR* dir = opendir(path);

	if(dir)
		return true;
	return false;
}

void create_directory(char* path)
{
	mkdir(path, 0755);
}


char* receive(int i)
{
	char* serialized_message = new char[SIZE + 8];

	memset(serialized_message, 0, SIZE + 8);

	int S = 0;

	int bytes_received = recv(i, serialized_message, SIZE + 8, 0);

	S = S + bytes_received;

	while(S != SIZE + 8)
	{
		int bytes_received = recv(i, serialized_message + S + 1, SIZE + 8, 0);
		S = S + bytes_received;
	}

	return serialized_message;
}

void receive_file(int i)
{
	char* serialized_message = new char[SIZE + 8];
	memset(serialized_message, 0, SIZE + 8);

	serialized_message = receive(i);

	Message msg;

	msg.deserialize_message(serialized_message);

	// Get the last token, which is the file name
	char* last_token = new char[1000];

	// Rebuild the path and the adjacent directories
	char* token = strtok(msg.payload, "/");
	char* path = new char[1000];

	strcpy(path, ".");

	while(token != NULL)
	{		
		if(strstr(token, ".html") == NULL)
		{
			strcat(path, "/");
			strcat(path, token);

			if(!exists_directory(path))
			{
				create_directory(path);
			}
		}

		strcpy(last_token, token);
		token = strtok(NULL, "/");	
	}

	strcat(path, "/");
	strcat(path, last_token);

	// Open the new file
	int write_fd = open(path, O_CREAT | O_WRONLY, S_IRGRP | S_IROTH | S_IRUSR);

	// Prepare the buffer
	memset(serialized_message, 0, SIZE + 8);

	// Get the message from the client
	serialized_message = receive(i);

	Message new_message;

	// Deserialize the message
	new_message.deserialize_message(serialized_message);

	// Write the entire content received from the client
	write(write_fd, new_message.payload, new_message.size);

	delete[] serialized_message;
	delete[] last_token;
	delete[] token;
	delete[] path;

	close(write_fd);
}

int main(int argc, char** argv)
{
	char port[5];
	
	string log_file;
	
	for(int i = 1; i<argc; i++) 
	{
		if(strcmp(argv[i], "-e") == 0)
			everything = true;

		if(strcmp(argv[i], "-r") == 0)
			recursive = true;

		if(strcmp(argv[i], "-o") == 0)
			log_file = argv[i + 1];

		if(strcmp(argv[i], "-p") == 0)
			strcpy(port, argv[i + 1]);
	}

	initialize_server(port);
	
	while(1) 
	{
		_tmp_fds = _read_fds;

		int select_result = select(_max_fd_range + 1, &_tmp_fds, NULL, NULL, NULL);
		is_valid(select_result, "select");

		if(select_result != 0)
		{
			for(int i = 0; i<=_max_fd_range; i++) 
			{
				if(FD_ISSET(i, &_tmp_fds))
				{
					if(i == _server_socket) 
					{
						accept_handler();	// Accept the connection from a new client
					}

					else if(i == 0) 		// STDIN message
					{
						char brute_buffer[10000];
						cin.getline(brute_buffer, 10000);

						string buffer(brute_buffer);

						if(buffer.compare("status") == 0) 
						{
							//print_status();
						}
						else if(buffer.compare("exit") == 0)
						{
							//send_exit_message();
						}
						else if(buffer.substr(0, 8).compare("download") == 0)
						{
							//download command
							if(recursive)
							{
							 	links.push_back(Link(2, buffer.substr(9, string::npos).c_str()));
							}
							else
								links.push_back(Link(0, buffer.substr(9, string::npos).c_str()));

							 Client client = online_clients.front();
							 online_clients.pop();

							 send_link(client.file_descriptor, links.back());
							 links.pop_back();

							 online_clients.push(client);
						}
					}

					else  
					{
						receive_file(i);	// Get a file from a client

						if(recursive)		// Get the list of links
						{
							Message msg;

							char* serialized_message = new char[SIZE + 8];

							memset(serialized_message, 0, SIZE + 8);

							serialized_message = receive(i);	

							msg.deserialize_message(serialized_message);

							int nivel = msg.type;

							char* token = new char[10];

							token = strtok(msg.payload, " ");

							while(token != NULL) 
							{
								links.push_back(Link(nivel, token));	// Add the links in the links list
								token = strtok(NULL, " ");
							}

							Client client = online_clients.front();		// Send a link to a new client
							online_clients.pop();

							send_link(client.file_descriptor, (*links.begin()));
							links.erase(links.begin());

							online_clients.push(client);
						}
					}
				}
			}
		}
	}
}