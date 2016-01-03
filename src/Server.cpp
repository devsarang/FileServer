/*
 * Server.cpp
 *
 *  Created on: Sep 24, 2015
 *      Author: sarang
 *
 *      The server class handles the functionalities that server provides. Also has a command interprator and always
 *      listens for connection from clients.
 */
#include <cstring>
#include "../inc/Server.h"
#include <cstdlib>
#include "../inc/Constants.h"
#include <iostream>
#include <cstdio>
#include <ifaddrs.h>
#include <unistd.h>

Server::Server() {
	clientsConnected = 0;
	ser_socket_fd = 0;
	max_fd = 0;
	help = 0;
	socket_iterator = 0;
	FD_ZERO(&master_fd_set);
}

Server::~Server() {
}

int Server::serverInitialize(const char * port_no)
{
	help = new Help();
	strcpy(port,port_no);
	ser_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	max_fd = ser_socket_fd;
	if (ser_socket_fd < 0)
		return 1;
	struct sockaddr_in serv_address;
	memset(&serv_address, 0, sizeof(serv_address));
	serv_address.sin_family = AF_INET;
	serv_address.sin_addr.s_addr = INADDR_ANY;
	serv_address.sin_port = htons(atoi(port));
	if (bind(ser_socket_fd, (struct sockaddr *) &serv_address,
			sizeof(serv_address)) == -1) {
		close(ser_socket_fd);
		return 1;
	}

	struct ifaddrs *ifaddr, *tmp;

	if (getifaddrs(&ifaddr) == -1)
	{
		std::cout<<"Failed to get self address"<<std::endl;
	}
	tmp = ifaddr;
	while (tmp)
	{
		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET && (!strcmp(tmp->ifa_name,INTERFACE_NAME1)||!strcmp(tmp->ifa_name,INTERFACE_NAME2)))
		{
			struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
			strcpy(serveraddr, inet_ntoa(pAddr->sin_addr));
			break;
		}
		tmp = tmp->ifa_next;
	}

	freeifaddrs(ifaddr);
	strcpy(hostname, std::getenv("HOSTNAME"));
	list.addAddress(1,serveraddr,hostname,port_no);

	if (listen(ser_socket_fd, BACKLOG) == -1)
		return 1;
	return 0;
}

int Server::serverRun() {
	char command[CMD_LEN];
	char command_arg[CMD_SIZE][CMD_LEN];
	FD_SET(ser_socket_fd, &master_fd_set);
	FD_SET(fileno(stdin), &master_fd_set);
	while (1)
	{
		active_fd_set = master_fd_set;
		if (select(max_fd+1, &active_fd_set, NULL, NULL, NULL) < 0)
		 {
		      std::cout<< "ERROR in select"<<std::endl;
		      std::cout<<"SER>"<<std::flush;
		 }

		if (FD_ISSET(fileno(stdin), &active_fd_set))
		{
			gets(command);
			char* token = strtok(command, " ");
			int i = 0;
			while (token)
			{
				strcpy(command_arg[i++], token);
				token = strtok(NULL, " ");

			}
			switch (commandInterpretor(command_arg[0]))
			{
			case HELP:
				help->displayUsageServer();
				break;

			case CREATOR:
				std::cout << "Full Name : Sarang Dev" << std::endl;
				std::cout << "UBIT Name : sarangde" << std::endl;
				std::cout << "UB Email Address : sarangde@buffalo.edu"<<std::endl;
				break;

			case DISPLAY:
				std::cout << "Hostname : "<< hostname<<std::endl;
				std::cout << "Local IP Address : "<< serveraddr<<std::endl;
				std::cout << "Local Port No : "<< port<<std::endl;
				break;

			case LIST:
				list.printAddressList();
				break;

			case SYNC:
				if(1 == sendSyncToAll())
					std::cout<<"Error in sending SYNC to all clients"<<std::endl;
				break;

			case QUIT:
				std::cout << "Terminating the process"<<std::endl;
				serverQuit();
				break;

			default:
				std::cout << "Wrong command entered"<<std::endl;
				break;
			}
			if (commandInterpretor(command) == QUIT)
				break;
			FD_CLR(fileno(stdin), &active_fd_set);
		}
		if (FD_ISSET(ser_socket_fd, &active_fd_set))
		{
			int new_socket_fd;
			unsigned int size = sizeof(clientname);
			new_socket_fd = accept(ser_socket_fd,(struct sockaddr *) &clientname, &size);
			if (new_socket_fd < 0)
			{
				std::cout << "accept error" << std::endl;
				exit(EXIT_FAILURE);
			}
			else
			{
				char cli_hostname[HOSTNAME_LEN];
				getnameinfo((struct sockaddr *) &clientname, sizeof(struct sockaddr), cli_hostname,sizeof(cli_hostname), NULL, 0, NI_NAMEREQD);
				char con_port[PORT_LEN];
				int result = recv(new_socket_fd, con_port, PORT_LEN+1, 0);
				if(result <= 0)
				{
					std::cout<<"Error in receiving port from client"<<std::endl;
				}
				list.addAddress(socket_iterator+2,inet_ntoa(clientname.sin_addr),cli_hostname,con_port);
				socket_list[socket_iterator++] = new_socket_fd;
				std::cout<<"New client registered IP : "<<inet_ntoa(clientname.sin_addr)<<" Port : "<<con_port<<std::endl;
				std::cout<<"The list has been updated :"<<std::endl;
				list.printAddressList();
				if(1 == sendListToAll())
					std::cout<<"Error in sending updated list to all clients"<<std::endl;
			}
			if (new_socket_fd > max_fd)
				max_fd = new_socket_fd;
			FD_SET(new_socket_fd,&master_fd_set);
			FD_CLR(ser_socket_fd,&active_fd_set);
		}
		char buffer[1024];
		for (int i = 0; i <= max_fd; i++)
		{
			if (FD_ISSET(i, &active_fd_set))
			{
				int res_count = recv(i, buffer, 1024, 0);
				if (res_count <= 0)
				{
					int close_id;
					for(int j=0;j<socket_iterator;j++)
					{
						if(i == socket_list[j])
						{
							close_id = j+2;
							socket_list[j] = -1;
							break;
						}
					}
					std::cout <<"Connection terminated from id:"<<close_id<<std::endl;
					close(i);
					FD_CLR(i,&master_fd_set);
					list.deleteAddress(close_id);
					std::cout<<"The list has been updated :"<<std::endl;
					list.printAddressList();
					if(1 == sendListToAll())
						std::cout<<"Error in sending updated list to all clients"<<std::endl;
				}
				else
				{
					char requestCmd[CMD_LEN];
					strcpy(requestCmd,buffer);
					if(!strcmp(requestCmd,"SYNC"))
					{
						if(1 == sendSyncToAll())
							std::cout<<"Error in sending SYNC to all clients"<<std::endl;
					}
				}
			}
		}
	}
	return 0;
}

command_code Server::commandInterpretor(char *string)
{
	int i = 0;
	while (string[i])
	{
		string[i] = tolower(string[i]);
		i++;
	}

	if (!strcmp(string, "help"))
		return HELP;
	if (!strcmp(string, "creator"))
		return CREATOR;
	if (!strcmp(string, "quit"))
		return QUIT;
	if (!strcmp(string, "display"))
		return DISPLAY;
	if (!strcmp(string, "list"))
		return LIST;
	if (!strcmp(string, "sync"))
		return SYNC;
	return UNKNOWN;
}

int Server::serverQuit()
{
	for (int i = 0; i < socket_iterator; i++)
	{
		close (socket_list[i]);
	}
	return 0;
}

int Server::sendListToAll()
{
	char stream[1024];
	list.serialize(stream);
	for (int i = 0; i < socket_iterator; i++)
	{
		if(socket_list[i] != -1)
			if(-1 == send(socket_list[i],stream,sizeof(stream),0))
				return 1;
	}
	return 0;
}

int Server::sendSyncToAll()
{
	for (int i = 0; i < socket_iterator; i++)
	{
		if (socket_list[i] != -1)
			if (-1 == send(socket_list[i], "SYNC", 5, 0))   //notify all the clients that SYNC has been executed
				return 1;
	}
	return 0;
}
