#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstdlib>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdlib> 
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdio>
#include "Message.h"

using namespace std;

int _client_socket;
char* global_site;
char* intermediate_path;

struct sockaddr_in _serv_addr;

void is_valid(int connection_object, std::string entity) 
{
	if(connection_object < 0) 
		cout << "Failure detected at: " << entity <<"\n";
}

void init_connection_parameters(char* ip, char* port)
{
	_serv_addr.sin_family = AF_INET;
    _serv_addr.sin_port = htons(atoi(port));
    inet_aton(ip, &_serv_addr.sin_addr);
}

void initialize_connection(char* ip, char* port)
{
	init_connection_parameters(ip, port);

	_client_socket = socket(AF_INET, SOCK_STREAM, 0);
	is_valid(_client_socket, "socket");

	int connect_result = connect(_client_socket,(struct sockaddr*) &_serv_addr,sizeof(_serv_addr));
	is_valid(connect_result, "connect");
}

// Get the server IP (DNS request)

struct in_addr get_ip(char full_path[])
{
	char site[10000];

	int i, j;

	// site will store the website name
	for(i = 0; i<strlen(full_path); i++)
	{
		if(full_path[i] != '/')
			site[i]  = full_path[i];
		else
		{
			site[i] = '\0';
			break;
		}
	}

	// Allign j at the last '/' from the path
	for(j = strlen(full_path) - 1; j>=0; j--)
	{
		if(full_path[j] == '/')
			break;
	}

	// Intermediate path will store the intermediate path of the html file
	intermediate_path = new char[10000];
	global_site = new char[10000];

	strncpy(intermediate_path, full_path + i, j - i + 1);
	intermediate_path[j - i + 1] = '\0'; 

	strcpy(global_site, site);

	struct hostent* object = gethostbyname(site);

	if(object != NULL)
	{
		struct in_addr* xx = (struct in_addr*)(object->h_addr);
		struct in_addr yy;
		yy.s_addr = xx->s_addr;

		cout<<inet_ntoa(yy)<<endl;	
		return yy;
	}
	else
	{
		cout<<"DNS request 404"<<endl;
	}

	struct in_addr addr;

	return addr;
}


// Check if the website has "http" or not
bool has_http(char site[])
{
	if(site[0] == 'h' && 
	   site[1] == 't' && 
	   site[2] == 't' && 
	   site[3] == 'p')
		return true;

	return false;
}

void send_command(int sockfd, char sendbuf[]) {

  int nbytes;
  char CRLF[3];
  
  CRLF[0] = 13; CRLF[1] = 10; CRLF[2] = 0;
  strcat(sendbuf, CRLF);
  
  write(sockfd, sendbuf, strlen(sendbuf));
}

char* get_file(char* path)
{
	for(int i = 0; i<strlen(path); i++)
	{
		if(path[i] == '/')
		{
			return path + i;
		}
	}
}

// Get the name of the html file (the last token from the path)
char* get_html_name(char* path)
{
	char* token = strtok(path, "/");
	char* last_token = (char*)malloc(100 * sizeof(char));

	while(token != NULL)
	{
		strcpy(last_token, token);
		token = strtok(NULL, "/");	
	}

	return last_token;
}

long get_file_dimension(char* file_name)
{
   int fp = open(file_name, O_RDONLY);

   long sz = lseek(fp, 0, SEEK_END);
   
   close(fp);

   return sz;
}

// Remove the header of a HTTP response and returns the payload

char* remove_header(char* content, int size, int *new_size)
{
	for(int i = 0; i<=size; i++)
	{
		if(content[i] == '\r' &&
			content[i + 1] == '\n' &&
			content[i + 2] == '\r' &&
			content[i + 3] == '\n')
		{
			(*new_size) = size - (i + 3);

			return content + i + 4;	
		}
	}
}

void dowload_content(char site[], int level)
{
	struct in_addr ip;

	char file[70];

	if(has_http(site)) 
	{
		ip = get_ip(site + 7);
		strcpy(file, get_file(site + 7));
	}
	else 
	{
		ip = get_ip(site);
		strcpy(file, get_file(site));
	}

	int sockfd;

	//_______________________________WEB_SERVER_CONNECT_________________________________________

	struct sockaddr_in servaddr;

    char sendbuf[SIZE + 8]; 
    memset(sendbuf, 0, SIZE + 8);

    char recvbuf[SIZE + 8];
    memset(recvbuf, 0, SIZE + 8);

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {

    	cout<<"Socket error"<<endl;
	  	exit(-1);
    }  

     /* Create the server's address */
	memset(&servaddr, 0, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr = ip;
	servaddr.sin_port = htons(80);

	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {

    	cout<<"Connection error"<<endl;
    	exit(-1);
  	}

  	strcpy(sendbuf, "GET ");
  	strcat(sendbuf, file);
  	strcat(sendbuf, " HTTP/1.0\n\n");

  	
  	// Download the file from the web server
  	send_command(sockfd, sendbuf);

  	int fd = open(get_html_name(strdup(file)), O_WRONLY | O_CREAT | O_TRUNC, S_IRGRP | S_IROTH | S_IRUSR);

  	int n = recv(sockfd, recvbuf, 500, 0);

  	// Write in chunks of 500 bytes (download) 
  	while(n)
  	{
  		write(fd, recvbuf, n);
  		n = recv(sockfd, recvbuf, 500, 0);
  	}

  	close(fd);
  	close(sockfd);

  	// Send the file name

  	memset(sendbuf, 0, SIZE + 8);

  	char* new_payload = new char[SIZE + 8];
  	memset(new_payload, 0, SIZE + 8);

  	strcpy(new_payload, global_site);
  	strcat(new_payload, file);

  	Message msg;

  	msg.type = 1;					// 1 = file_name token
  	msg.size = strlen(new_payload);

  	memcpy(msg.payload, new_payload, msg.size + 1);

	char* result = new char[SIZE + 8];

	// Serialize the file name
	result[3] = ((msg.type)>>24) & 0xFF;
	result[2] = ((msg.type)>>16) & 0xFF;
	result[1] = ((msg.type)>>8) & 0xFF;
	result[0] = (msg.type) & 0xFF;

	result[7] = ((msg.size)>>24) & 0xFF;
	result[6] = ((msg.size)>>16) & 0xFF;
	result[5] = ((msg.size)>>8) & 0xFF;
	result[4] = (msg.size) & 0xFF;

	memcpy(result + 8, msg.payload, SIZE);


  	send(_client_socket, result, SIZE + 8, 0);	// send the name

	// Send the content of the file

	int new_size = 0;
	long size = get_file_dimension(get_html_name(strdup(file)));

  	char* file_content = new char[size];
  	char* file_content_without_header = new char[size];

  	int file_fd = open(get_html_name(strdup(file)), O_RDONLY);

  	int read_file_size = read(file_fd, file_content, size);
  	

  	file_content_without_header = remove_header(file_content, read_file_size, &new_size);

  	Message new_message;

	new_message.type = 2;		// File-content
	new_message.size = new_size;

	memcpy(new_message.payload, file_content_without_header, new_message.size);


	// Serialize file-content
	memset(result, 0, SIZE + 8);

	result[3] = ((new_message.type)>>24) & 0xFF;
	result[2] = ((new_message.type)>>16) & 0xFF;
	result[1] = ((new_message.type)>>8) & 0xFF;
	result[0] = (new_message.type) & 0xFF;

	result[6] = ((new_message.size)>>16) & 0xFF;
	result[5] = ((new_message.size)>>8) & 0xFF;
	result[4] = (new_message.size) & 0xFF;

	memcpy(result + 8, new_message.payload, SIZE);

  	send(_client_socket, result, SIZE + 8, 0);	// Send content

  	close(file_fd);

  	//________________________________________END_SEND_FILE___________________________________________


  	if(level >= 2 && level <= 6)		// Recursive mode
  	{
  		int index = 0;

  		Message message;
  		message.type = level + 1;		// Raise recursion level


  		char* links = new char[size];			// The links that I send to the server
  		file_content = new char[SIZE + 8];

  		int file_fd = open(get_html_name(strdup(file)), O_RDONLY);


  		// Read nr_bytes from file
  		int nr_bytes = read(file_fd, file_content, size);

  		char* link = new char[SIZE];

  		// complete_link = the entire path
  		char* complete_link = new char[SIZE];

  		while(index != nr_bytes)
  		{

  			// If I hit that pattern
  			if(file_content[index] == '<' &&
  				file_content[index + 1] == 'a' &&
  				file_content[index + 2] == ' ' &&
  				file_content[index + 3] == 'h' &&
  				file_content[index + 4] == 'r' &&
  				file_content[index + 5] == 'e' &&
  				file_content[index + 6] == 'f' &&
  				file_content[index + 7] == '=' &&
  				file_content[index + 8] == '\"')
  			{

  				// index is immediately after the "
  				index = index + 9;

  				int index_copy = index;

  				// Check the link ending
  				while(file_content[index] != '>')
  				{
  					index++;
  				}

  				memset(link, 0, SIZE);

  				// Copy the link
  				strncpy(link, file_content + index_copy, index - index_copy);
  				link[strlen(link) - 1] = '\0';

  				// If the link doesn't contain : and is .html
  				if(strstr(link, ":") == NULL && strstr(link, ".html") != NULL) 
  				{

  					memset(complete_link, 0, SIZE);

  					// Assemble the entire path
  					strcpy(complete_link, global_site);
  					strcat(complete_link, intermediate_path);

  					// If the link starts with /, then I pass over it
  					if(link[0] == '/')
  						strcat(complete_link, link + 1);
  					else
  					{
  						strcat(complete_link, link);
  					}

  					// Links will be the container for the links list
  					strcat(links, complete_link);
  					strcat(links, " ");
	  			}
  			}

  			index++;
  		}

  		delete[] link;
  		delete[] complete_link;

  		message.size = strlen(links);
  		memcpy(message.payload, links, strlen(links));

  		// Serialize the message (The links list)
		memset(result, 0, SIZE + 8);

		result[3] = ((message.type)>>24) & 0xFF;
		result[2] = ((message.type)>>16) & 0xFF;
		result[1] = ((message.type)>>8) & 0xFF;
		result[0] = (message.type) & 0xFF;

		result[7] = ((message.size)>>24) & 0xFF;
		result[6] = ((message.size)>>16) & 0xFF;
		result[5] = ((message.size)>>8) & 0xFF;
		result[4] = (message.size) & 0xFF;

		memcpy(result + 8, message.payload, SIZE);

  		send(_client_socket, result, SIZE + 8, 0);	

  		close(file_fd);
  	}

  	// Delete the temporary file (the downoaded one)
  	unlink(get_html_name(strdup(file)));
}

int main(int argc, char** argv)
{
	char port[5];
	char IP[12];

	string log_file;

	for(int i = 1; i<argc; i++) 
	{
		if(strcmp(argv[i], "-o") == 0)
			log_file = argv[i + 1];

		if(strcmp(argv[i], "-a") == 0)
			strcpy(IP, argv[i+1]);

		if(strcmp(argv[i], "-p") == 0)
			strcpy(port, argv[i + 1]);
	}

	initialize_connection(IP, port);

	while(1)
	{
		char* serialized_message = new char[SIZE + 8];
		memset(serialized_message, 0, SIZE + 8);

		int bytes_received = recv(_client_socket, serialized_message, SIZE + 8, 0);

		Message message;

		message.deserialize_message(serialized_message);
		message.print_message();


		if(message.type == 0)	// I've received a link (simple mode)
		{
			dowload_content(message.payload, -1);
		}
		else if(message.type >= 2 && message.type <= 6) 	// I've received link (recursive mode)
		{ 
			dowload_content(message.payload, message.type);
		}
		
		delete[] serialized_message;
	}
}