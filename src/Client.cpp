/*
 * Client.cpp
 *
 *  Created on: Sep 24, 2015
 *      Author: sarang
 *
 *      The client class handles the functionalities of the client program and also connects to the server and other clients. It also
 *      listens for connection from other peers. It supports file download and transfer using GET and PUt commands.
 */

#include "../inc/Client.h"
#include "../inc/Constants.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <ifaddrs.h>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <time.h>

Client::Client() {
	peersConnected = 0;
	cli_socket_fd = 0;
	ser_socket_fd = 0;
	max_fd = 0;
	help = 0;
	registered = false;
	peerInfoList_iter = -1;
	FD_ZERO(&master_fd_set);
}

Client::~Client() {
}

int Client::clientInitialize(const char * port_no) {
	help = new Help();
	strcpy(port, port_no);
	cli_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	max_fd = cli_socket_fd;
	if (cli_socket_fd < 0)
		return 1;
	struct sockaddr_in cli_address;
	memset(&cli_address, 0, sizeof(cli_address));
	cli_address.sin_family = AF_INET;
	cli_address.sin_addr.s_addr = INADDR_ANY;
	cli_address.sin_port = htons(atoi(port));
	if (bind(cli_socket_fd, (struct sockaddr *) &cli_address,
			sizeof(cli_address)) == -1) {
		close(cli_socket_fd);
		return 1;
	}

	struct ifaddrs *ifaddr, *tmp;

	if (getifaddrs(&ifaddr) == -1) {
		std::cout << "Failed to get self address" << std::endl;
	}
	tmp = ifaddr;
	while (tmp) {
		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET
				&& (!strcmp(tmp->ifa_name, INTERFACE_NAME1)
						|| !strcmp(tmp->ifa_name, INTERFACE_NAME2))) {
			struct sockaddr_in *pAddr = (struct sockaddr_in *) tmp->ifa_addr;
			strcpy(selfaddr, inet_ntoa(pAddr->sin_addr));
			break;
		}
		tmp = tmp->ifa_next;
	}

	freeifaddrs(ifaddr);
	strcpy(hostname, std::getenv("HOSTNAME"));
	char hostname_copy[HOSTNAME_LEN];
	strcpy(hostname_copy,hostname);
	char *token = strtok(hostname_copy,".");					//setting peer filename for SYNC to be <server>.txt
	strcat(token, ".txt");
	strcpy(sync_filename,token);

	if (listen(cli_socket_fd, BACKLOG) == -1)
		return 1;
	return 0;
}

int Client::terminateConnection(int id) {
	if (id == 1)
		close(ser_socket_fd);
	else
		close(peerInfoList[id - 2].socket);
	peerList.deleteAddress(id);
	return 0;
}

int Client::peerConnect(const char* server_address, const char* port_no) {
	int con_socket_fd;
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(server_address, port_no, &hints, &res) != 0) {
		return 1;
	}

	con_socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (con_socket_fd < 0) {
		std::cout << "error in socket connection" << std::endl;
	}

	if (-1 == connect(con_socket_fd, res->ai_addr, res->ai_addrlen)) {
		close(con_socket_fd);
		std::cout << "client not connected" << std::endl;
	}
	if (-1 == send(con_socket_fd, port, sizeof(port), 0)) {
		std::cout << "Error in sending message to client" << std::endl;
	}
	struct sockaddr_in sa;
	inet_pton(AF_INET,
			inet_ntoa(((struct sockaddr_in*) res->ai_addr)->sin_addr),
			&sa.sin_addr);
	sa.sin_family = AF_INET;
	char ser_hostname[HOSTNAME_LEN],ser_hostname_copy[HOSTNAME_LEN];

	int return_val = getnameinfo((struct sockaddr *) &sa,
			sizeof(struct sockaddr), ser_hostname, sizeof(ser_hostname), NULL,
			0, NI_NAMEREQD);
	if (return_val != 0) {
		std::cout << gai_strerror(return_val) << std::endl;
	}
	peerInfoList_iter++;
	peerInfoList[peerInfoList_iter].socket = con_socket_fd;
	peerInfoList[peerInfoList_iter].data_put = 0;
	peerInfoList[peerInfoList_iter].data_req = 0;
	peerInfoList[peerInfoList_iter].active = true;

	strcpy(ser_hostname_copy,ser_hostname);
	char *token = strtok(ser_hostname_copy,".");					//setting peer filename for SYNC to be <server>.txt
	strcat(token, ".txt");
	strcpy(peerInfoList[peerInfoList_iter].filename,token);


	peerList.addAddress(peerInfoList_iter + 2,inet_ntoa(((struct sockaddr_in*) res->ai_addr)->sin_addr),ser_hostname, port_no);

	std::cout << "Connected to peer IP : "
			<< inet_ntoa(((struct sockaddr_in*) res->ai_addr)->sin_addr)
			<< " Port : " << port_no << std::endl;
	FD_SET(con_socket_fd, &master_fd_set);
	if (con_socket_fd > max_fd)
		max_fd = con_socket_fd;
	freeaddrinfo(res);
	return 0;
}

int Client::clientServerConnect(const char* server_address,
		const char* port_no) {
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(server_address, port_no, &hints, &res) != 0) {
		return 1;
	}

	ser_socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (ser_socket_fd < 0) {
		std::cout << "error in socket connection" << std::endl;
	}

	if (-1 == connect(ser_socket_fd, res->ai_addr, res->ai_addrlen)) {
		close(ser_socket_fd);
		std::cout << "client not connected" << std::endl;
		return 1;
	}
	if (-1 == send(ser_socket_fd, port, sizeof(port), 0))
	{
		std::cout << "Error in sending message to client" << std::endl;
		return 1;
	}
	struct sockaddr_in sa;
	inet_pton(AF_INET, inet_ntoa(((struct sockaddr_in*) res->ai_addr)->sin_addr), &sa.sin_addr);
	sa.sin_family = AF_INET;
	char ser_hostname[HOSTNAME_LEN];

	int return_val = getnameinfo((struct sockaddr *)&sa, sizeof(struct sockaddr), ser_hostname,sizeof(ser_hostname), NULL, 0, NI_NAMEREQD);
	if(return_val != 0)
	{
		std::cout<<gai_strerror(return_val)<<std::endl;
		return 1;
	}
	peerList.addAddress(1,
			inet_ntoa(((struct sockaddr_in*) res->ai_addr)->sin_addr),
			ser_hostname, port_no);
	std::cout << "Connected to the server" << std::endl;
	freeaddrinfo(res);

	FD_SET(ser_socket_fd, &master_fd_set);
	if (ser_socket_fd > max_fd)
		max_fd = ser_socket_fd;
	registered = true;
	return 0;
}

int Client::clientRun()
{
	char command[CMD_LEN];
	char command_arg[CMD_SIZE][CMD_LEN];
	FD_SET(cli_socket_fd, &master_fd_set);
	FD_SET(fileno(stdin), &master_fd_set);
	while (1)
	{
		active_fd_set = master_fd_set;
		if (select(max_fd + 1, &active_fd_set, NULL, NULL, NULL) < 0)
		{
			std::cout << "ERROR in select" << std::endl;
		}

		if (FD_ISSET(fileno(stdin), &active_fd_set)) {
			gets(command);
			char* token = strtok(command, " ");
			int i = 0;
			while (token) {
				strcpy(command_arg[i++], token);
				token = strtok(NULL, " ");
			}
			switch (commandInterpretor(command_arg[0])) {
			case HELP:
				help->displayUsageClient();
				break;

			case CREATOR:
				std::cout << "Full Name : Sarang Dev" << std::endl;
				std::cout << "UBIT Name : sarangde" << std::endl;
				std::cout << "UB Email Address : sarangde@buffalo.edu"
						<< std::endl;
				break;

			case DISPLAY:
				std::cout << "Hostname : " << hostname << std::endl;
				std::cout << "Local IP Address : " << selfaddr << std::endl;
				std::cout << "Local Port No : " << port << std::endl;
				break;

			case REGISTER:
				if (registered)
				{
					std::cout<< "You are already registered to the server."<< std::endl;
					break;
				}
				strcpy(serverIp, command_arg[1]);
				strcpy(serverPort, command_arg[2]);
				if (0 != clientServerConnect(serverIp, serverPort))
				{
					std::cout << "Client Not Registered. Try Again"
							<< std::endl;
				}
				break;
			case CONNECT:
				char peer_ip[HOSTNAME_LEN];
				char peer_port_no[PORT_LEN];
				strcpy(peer_ip, command_arg[1]);
				strcpy(peer_port_no, command_arg[2]);
				if (!registered) {
					std::cout
							<< "You are not registered to the server. First register and then CONNENCT"
							<< std::endl;
					break;
				}
				if(peerList.checkAddressExists(peer_ip))
				 {
				 std::cout<<"Connection Already Exists"<<std::endl;
				 break;
				 }
				if (!checkFromServerList(peer_ip)) {
					std::cout
							<< "The given address is not registered with Server"
							<< std::endl;
					break;
				}
				if(!strcmp(peer_ip,selfaddr)||!strcmp(peer_ip,hostname))
				 {
				 std::cout<<"Do not self connect. Not valid."<<std::endl;
				 break;
				 }
				if (0 != peerConnect(peer_ip, peer_port_no)) {
					std::cout << "Client Not Registered. Try Again"
							<< std::endl;
				}
				break;

			case LIST:
				peerList.printAddressList();
				break;

			case TERMINATE:
				int term_id;
				term_id = command_arg[1][0] - '0';
				if (!peerList.checkIdExists(term_id)) {
					std::cout << "Id " << term_id
							<< " does not exist in the list" << std::endl;
					break;
				}
				if (!terminateConnection(term_id))
					std::cout << "Connection with " << term_id
							<< " terminated successfully" << std::endl;
				else
					std::cout << "Connection with " << term_id
							<< " not terminated " << std::endl;
				break;

			case GET:
				int get_id;
				get_id = atoi(command_arg[1]);
				char file[100];
				strcpy(file, command_arg[2]);
				if (!peerList.checkIdExists(get_id)) {
					std::cout << "Id " << get_id
							<< " does not exist in the list" << std::endl;
					break;
				}
				if (1 == requestFile(get_id, file)) {
					std::cout << "Request for file failed" << std::endl;
				}

				break;

			case PUT:
				int put_id;
				put_id = atoi(command_arg[1]);
				char filename[100];
				strcpy(filename, command_arg[2]);
				if (put_id == 1)
				{
					std::cout<<"Cannot PUT files to the server"<<std::endl;
					break;
				}
				if (!peerList.checkIdExists(put_id)) {
					std::cout << "Id " << put_id
							<< " does not exist in the list" << std::endl;
					break;
				}
				if (1 == putFile(put_id, filename)) {
					std::cout << "Could not send the file" << std::endl;
				}

				break;

			case SYNC:
				if(-1 == send(ser_socket_fd,"SYNC",5,0))    					//notify the server that SYNC has been executed
					std::cout<<"Error in sending SYNC signal to Server"<<std::endl;
				break;
			case QUIT:
				std::cout << "Terminating the process" << std::endl;
				clientQuit();
				break;

			default:
				std::cout << "Wrong command entered" << std::endl;
				break;
			}
			if (commandInterpretor(command) == QUIT)
				break;
			FD_CLR(fileno(stdin), &active_fd_set);
		}
		if (FD_ISSET(cli_socket_fd, &active_fd_set))    //listening socket set, we have got a new connection
		{
			if (peerList.calcSize() < MAX_PEER + 1)
			{
				int new_socket_fd;
				struct sockaddr_in peername;
				unsigned int size = sizeof(peername);
				new_socket_fd = accept(cli_socket_fd,
						(struct sockaddr *) &peername, &size);
				if (new_socket_fd < 0) {
					std::cout << "accept error" << std::endl;
					exit(EXIT_FAILURE);
				} else {
					char cli_hostname[HOSTNAME_LEN],cli_hostname_copy[HOSTNAME_LEN];
					getnameinfo((struct sockaddr *) &peername,
							sizeof(struct sockaddr), cli_hostname,
							sizeof(cli_hostname), NULL, 0, NI_NAMEREQD);
					char con_port[PORT_LEN];
					int result = recv(new_socket_fd, con_port, PORT_LEN + 1, 0);
					if (result <= 0) {
						std::cout << "Error in receiving port from peer"
								<< std::endl;
					}
					peerInfoList_iter++;
					peerInfoList[peerInfoList_iter].socket = new_socket_fd;
					peerInfoList[peerInfoList_iter].data_put = 0;
					peerInfoList[peerInfoList_iter].data_req = 0;
					peerInfoList[peerInfoList_iter].active = true;
					strcpy(cli_hostname_copy,cli_hostname);
					char *token = strtok(cli_hostname_copy,".");					//setting peer filename for SYNC to be <server>.txt
					strcat(token, ".txt");
					strcpy(peerInfoList[peerInfoList_iter].filename,token);

					peerList.addAddress(peerInfoList_iter + 2,inet_ntoa(peername.sin_addr), cli_hostname,con_port);

					std::cout << "New peer connected IP : "
							<< inet_ntoa(peername.sin_addr) << " Port : "
							<< con_port << std::endl;
				}
				if (new_socket_fd > max_fd)
					max_fd = new_socket_fd;
				FD_SET(new_socket_fd, &master_fd_set);
				FD_CLR(cli_socket_fd, &active_fd_set);
			} else {
				std::cout << "Maximum of 3 peers can be connected. Try Later.."
						<< std::endl;
			}
		}
		if (FD_ISSET(ser_socket_fd, &active_fd_set))   // if the socket from server is set server will send something
		{
			char buffer[1024];
			int res_count = recv(ser_socket_fd, buffer, 1024, 0);
			if (res_count <= 0)
			{
				std::cout << "Connection from server broken" << std::endl;
				close(ser_socket_fd);
			}
			else if(!strcmp(buffer,"SYNC"))
			{
				if (1 == executeSync())
				{
					std::cout << "Error in SYNC,cannot get the Files"<< std::endl;
				}
			}
			else
			{
				std::cout << "Server updated the list" << std::endl;
				serverList.clearAll();
				serverList.deSerialize(buffer);
				serverList.printAddressList();
			}
			FD_CLR(ser_socket_fd, &active_fd_set);
		}
		char buffer[CMD_LEN];
		for (int i = 0; i <= max_fd; i++)		//we have data from other sockets or it has terminated
		{
			if (FD_ISSET(i, &active_fd_set))
			{
				int res_count = recv(i, buffer, 1024, 0); // Process data from socket i
				if (res_count <= 0) 			// case when the socket has been closed from other end
				{
					int close_id;
					for (int j = 0; j < peerInfoList_iter; j++)
					{
						if (i == peerInfoList[j].socket)
						{
							close_id = j + 2;
							peerInfoList[j].active = false;
							break;
						}
					}
					std::cout << "Connection terminated from id:" << close_id
							<< std::endl;
					close(i);
					peerList.deleteAddress(close_id);
					std::cout << "The Peer list has been updated :"
							<< std::endl;
					peerList.printAddressList();
					FD_CLR(i, &master_fd_set);
				}
				else										//some data on the socket
				{
                                        char requestCmd[CMD_LEN];
					char argList[ARG_LEN];
					char *token1 = strtok(buffer, " ");
					if(token1)
						strcpy(requestCmd, token1);
					char *token2 = strtok(NULL, " ");
					if(token2)
						strcpy(argList, token2);
					if (!strcmp(requestCmd, "GET")) 		//we have a GET request
					{
						if (1 == sendFile(i, argList))
						{
							std::cout << "Error in sending the File"
									<< std::endl;
						}
					}
					if (!strcmp(requestCmd, "PUT")) 		//we have a PUT request
					{
						if (1 == recvFile(i, argList))
						{
							std::cout << "Error in receiving the File"
									<< std::endl;
						}
					}
				}
			}
		}
	}
	return 0;
}

int Client::clientQuit()
{
	close(ser_socket_fd);        //close the server fd
	close(cli_socket_fd);		//close client listening fd

	for (int i = 0; i < peerInfoList_iter; i++)   //closing other peer fd
			{
		close(peerInfoList[i].socket);
	}
	return 0;
}

command_code Client::commandInterpretor(char *string) {
	int i = 0;
	while (string[i]) {
		string[i] = tolower(string[i]);
		i++;
	}

	if (!strcmp(string, "help"))
		return HELP;
	if (!strcmp(string, "creator"))
		return CREATOR;
	if (!strcmp(string, "quit"))
		return QUIT;
	if (!strcmp(string, "register"))
		return REGISTER;
	if (!strcmp(string, "display"))
		return DISPLAY;
	if (!strcmp(string, "connect"))
		return CONNECT;
	if (!strcmp(string, "list"))
		return LIST;
	if (!strcmp(string, "terminate"))
		return TERMINATE;
	if (!strcmp(string, "sync"))
		return SYNC;
	if (!strcmp(string, "get"))
		return GET;
	if (!strcmp(string, "put"))
		return PUT;
	return UNKNOWN;
}

int Client::checkFromServerList(const char *peerIp)
{
	if (serverList.checkAddressExists(peerIp))
		return 1;
	return 0;
}

int Client::requestFile(int id, const char * filename)
{
	time_t begin,end;
	struct tm * timeinfo;
	char time_buf [10];
	char buf[50];
	strcpy(buf, "GET ");
	strcat(buf, filename);
	send(peerInfoList[id - 2].socket, buf, sizeof(buf), 0);
	std::cout << "Requesting the file " << filename << " from id " << id
			<< std::endl;
	long value, size;
	recv(peerInfoList[id - 2].socket, &value, sizeof(value), 0);
	size = ntohl(value);
	if (size > 0)
	{
		std::cout << "Downloading the file " << filename << " from id " << id
				<< std::endl;
		begin = time(NULL);
		timeinfo = localtime (&begin);
		strftime (time_buf,10,"%H:%M:%S",timeinfo);
		std::cout << "Start Time : "<<time_buf<<std::endl;
		char buffer[BUFFER_SIZE];
		std::ofstream file;
		file.open(filename);
		long written = 0;
		while (size > 0)
		{
			int num;

			num = recv(peerInfoList[id - 2].socket, buffer, BUFFER_SIZE, 0);
			if(num <= 0)
				return false;
			file.write(buffer, num);
			written += num;
			size -= num;
		}
		peerInfoList[id - 2].data_req += written;
		std::cout << "Download Complete" << std::endl;
		end = time(NULL);
		timeinfo = localtime (&end);
		strftime (time_buf,10,"%H:%M:%S",timeinfo);
		std::cout <<" Time End : "<<time_buf<<std::endl;
		std::cout <<" Time Taken : "<<difftime(end,begin)<<std::endl;
		file.close();
	}
	else if(size == 0)
	{
		std::cout <<"The file has been refused by peer as it was not a valid request or"
				"The file is of 0 size, hence not downloading it"
				<< std::endl;
		return 1;
	}
	return 0;
}

int Client::sendFile(int socket_fd, const char * filename)
{
	time_t begin,end;
	struct tm * timeinfo;
	char time_buf [10];
	char filename_copy[FILENAME_LEN];
	strcpy(filename_copy,filename);
	if(strchr(filename_copy,'/') != NULL)
	{
		std::cout<<"Cannot send the file from any other directory. Hence canceling the upload"<<std::endl;
		long value = 0;
		send(socket_fd, &value, sizeof(value), 0);
		return 1;
	}

	std::ifstream file(filename);
	if (!file.good())
	{
		std::cout << "The requested file doesn't exist or is inaccessible"
				<< std::endl;
		long value = 0;
		send(socket_fd, &value, sizeof(value), 0);
		return 1;
	}
	char buffer[BUFFER_SIZE];
	file.seekg(0, file.end);
	long size = file.tellg();
	//send the filesize first
	long value = htonl(size);
	send(socket_fd, &value, sizeof(value), 0);

	file.seekg(0, file.beg);
	if (size > 0) {
		std::cout << "Uploading the file " << filename << std::endl;
		begin = time(NULL);
		timeinfo = localtime (&begin);
		strftime (time_buf,10,"%H:%M:%S",timeinfo);
		std::cout << "Start Time : "<<time_buf<<std::endl;
		begin= time(NULL);
		end = time(NULL);
		while (size > 0) {
			long num;
			if (size > sizeof(buffer))
				num = sizeof(buffer);
			else
				num = size;
			file.read(buffer, num);
			if (-1 == send(socket_fd, buffer, num, 0))
				return false;
			size -= num;
		}
		std::cout << "File uploaded successfully" << std::endl;
		end = time(NULL);
		timeinfo = localtime (&end);
		strftime (time_buf,10,"%H:%M:%S",timeinfo);
		std::cout <<" Time End : "<<time_buf<<std::endl;
		std::cout <<" Time Taken : "<<difftime(end,begin)<<std::endl;
	}
	file.close();
	return 0;
}

int Client::putFile(int id, const char * filename)
{
	time_t begin,end;
	struct tm * timeinfo;
	char time_buf [10];
	std::ifstream file(filename);
	if (!file.good())
	{
		std::cout << "The requested file doesn't exist or is inaccessible"
				<< std::endl;
		return 1;
	}
	char buf[FILENAME_LEN];
	strcpy(buf, "PUT ");
	strcat(buf, filename);
	send(peerInfoList[id - 2].socket, buf, sizeof(buf), 0);
	char buffer[BUFFER_SIZE];
	file.seekg(0, file.end);
	long size = file.tellg();
	//send the filesize first
	long value = htonl(size);
	send(peerInfoList[id - 2].socket, &value, sizeof(value), 0);
	char return_val;
	recv(peerInfoList[id - 2].socket,&return_val, 1, 0);
	if(return_val == 'N')
	{
		std::cout<<"The peer has refused to accept the file"<<std::endl;
		return 1;
	}
	file.seekg(0, file.beg);
	if (size > 0)
	{
		std::cout << "Uploading the file " << filename << std::endl;
		begin = time(NULL);
		timeinfo = localtime (&begin);
		strftime (time_buf,10,"%H:%M:%S",timeinfo);
		std::cout << "Start Time : "<<time_buf<<std::endl;
		while (size > 0)
		{
			long num;
			if (size > sizeof(buffer))
				num = sizeof(buffer);
			else
				num = size;
			file.read(buffer, num);
			if (-1 == send(peerInfoList[id - 2].socket, buffer, num, 0))
				return false;
			size -= num;
		}
		std::cout << "File uploaded successfully" << std::endl;
		end = time(NULL);
		timeinfo = localtime (&end);
		strftime (time_buf,10,"%H:%M:%S",timeinfo);
		std::cout <<" Time End : "<<time_buf<<std::endl;
		std::cout <<" Time Taken : "<<difftime(end,begin)<<std::endl;
	}
	file.close();
	return 0;
}

int Client::recvFile(int socket_fd, const char * filename)
{
	time_t begin,end;
	struct tm * timeinfo;
	char time_buf [10];

	long value, size;
	recv(socket_fd, &value, sizeof(value), 0);
	size = ntohl(value);
	int iter = 0;
	for (int j = 0; j < peerInfoList_iter; j++)
	{
		if (socket_fd == peerInfoList[j].socket)
		{
			iter = j;
			break;
		}
	}

	if(peerInfoList[iter].data_put + size > MAX_PUT_DATA)
	{
		std::cout<<"Quota for max file transfer from the peer exceeded"<<std::endl;
		char response = 'N';
		send(socket_fd,&response, sizeof(char), 0);
		return 1;
	}
	char response = 'Y';
	send(socket_fd,&response, sizeof(char), 0);
	if (size != 0)
	{
		char buffer[BUFFER_SIZE];
		std::ofstream file;
		char filename_copy[50];
		strcpy(filename_copy,filename);
		char *token = strtok(filename_copy,"/");
		char *lst_token;
		if(token == NULL)
			lst_token = filename_copy;
		while(token != NULL)
		{
			lst_token = token;
			token = strtok(NULL,"/");
		}
		file.open(lst_token);
		std::cout << "Receiving the file " << lst_token << std::endl;
		begin = time(NULL);
		timeinfo = localtime (&begin);
		strftime (time_buf,10,"%H:%M:%S",timeinfo);
		std::cout << "Start Time : "<<time_buf<<std::endl;	
	
		int written = 0;
		while (size > 0)
		{
			int num;

			num = recv(socket_fd, buffer, BUFFER_SIZE, 0);
			if(num <= 0)
				return false;

			file.write(buffer, num);
			written += num;
			size -= num;
		}
		peerInfoList[iter].data_put += written;
		std::cout << "Download Complete" << std::endl;
		end = time(NULL);
		timeinfo = localtime (&end);
		strftime (time_buf,10,"%H:%M:%S",timeinfo);
		std::cout <<" Time End : "<<time_buf<<std::endl;
		std::cout <<" Time Taken : "<<difftime(end,begin)<<std::endl;
		file.close();
	}
	else
	{
		std::cout << "The file is of 0 size, hence not downloading it"
				<< std::endl;
	}
	return 0;
}


int Client::executeSync()
{
	time_t begin,end;
	struct tm * timeinfo;
	char time_buf [10];
	long filesize[PEER_LIST_LEN];
	std::ofstream outFile[PEER_LIST_LEN];
	std::ifstream file(sync_filename);
	if (!file.good())
	{
		std::cout << "Sync file is missing"<< std::endl;
		return 1;
	}
	file.seekg(0, file.end);
	long file_size, size = file.tellg();
	file.seekg(0, file.beg);

	long value = htonl(size);
	for (int i = 0; i <= peerInfoList_iter; i++)
	{
		if (peerInfoList[i].active == true)
		{
			send(peerInfoList[i].socket, &value, sizeof(long), 0);
			recv(peerInfoList[i].socket, &file_size, sizeof(long), 0);
			filesize[i] = ntohl(file_size);
		}
	}
	for (int i = 0; i <= peerInfoList_iter; i++)
	{
		if (peerInfoList[i].active == true)
		{
			begin = time(NULL);
			timeinfo = localtime (&begin);
			strftime (time_buf,10,"%H:%M:%S",timeinfo);
			std::cout<<"Start "<<peerInfoList[i].filename<<" at "<<time_buf<<std::endl;
			outFile[i].open(peerInfoList[i].filename);
		}
	}
	char buffer_in[BUFFER_SIZE];
	char buffer_out[BUFFER_SIZE];
	bool flag = true;
	while(flag)
	{
		flag = false;
		long num_out;
		if (size > sizeof(buffer_out))
			num_out = sizeof(buffer_out);
		else
			num_out = size;
		memset(buffer_out, 0, sizeof(buffer_out));
		if (size > 0)
			file.read(buffer_out, num_out);
		for (int i = 0; i <= peerInfoList_iter; i++)
		{
			if (peerInfoList[i].active == true && size > 0)
			{
				send(peerInfoList[i].socket, buffer_out, num_out, 0);
			}
		}
		size -= num_out;

		for (int i = 0; i <= peerInfoList_iter; i++)
		{
			if (filesize[i] > 0)
			{
				int num_in = recv(peerInfoList[i].socket, buffer_in,sizeof(buffer_in), 0);
				outFile[i].write(buffer_in,num_in);
				filesize[i] -= num_in;
				if(filesize[i] == 0)
				{
					outFile[i].close();
					end = time(NULL);
					timeinfo = localtime (&end);
					strftime (time_buf,10,"%H:%M:%S",timeinfo);
					std::cout<<"End "<<peerInfoList[i].filename<<" at "<<time_buf<<std::endl;
				}
			}
		}
		for (int i = 0; i <= peerInfoList_iter; i++)
		{
			if (filesize[i] > 0 || size > 0)
				flag = true;
		}
	}
	return 0;
}

