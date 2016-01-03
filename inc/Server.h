/*
 * Server.h
 *
 *  Created on: Sep 21, 2015
 *      Author: sarang
 *
 *      Declaration for the Server Class
 */


#ifndef _SERVER_H
#define _SERVER_H

#include <iostream>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../inc/Constants.h"
#include "../inc/AddressList.h"
#include "../inc/Help.h"


class Server
{
	int clientsConnected;
	int ser_socket_fd;
	int socket_list[SOCK_LIST_LEN];
	int socket_iterator;
	struct sockaddr_in clientname;
	AddressList list;
	fd_set active_fd_set, master_fd_set;
	int max_fd;
	char serveraddr[IP_LEN];
	char hostname[HOSTNAME_LEN];
	char port[PORT_LEN];
	Help * help;

public:
	Server();
	virtual ~Server();
	int serverInitialize(const char *);
	int serverRun();
	command_code commandInterpretor(char *);
	int serverQuit();
	int sendListToAll();
	int sendSyncToAll();

};

#endif
