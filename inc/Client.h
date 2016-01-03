/*
 * Client.h
 *
 *  Created on: Sep 21, 2015
 *      Author: sarang
 *
 *      Declaration of the Client Class
 */


#ifndef CLIENT_H_
#define CLIENT_H_

#include <iostream>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../inc/Constants.h"
#include "../inc/AddressList.h"
#include "../inc/Help.h"

class Client
{
	int peersConnected;
	int cli_socket_fd;
	struct peerInfo
	{
		bool active ;
		long data_put;
		long data_req;
		int socket;
		char filename[FILENAME_LEN];
	};
	peerInfo peerInfoList[PEER_LIST_LEN];
	int peerInfoList_iter;
	char serverIp[HOSTNAME_LEN];
	char serverPort[PORT_LEN];
	int ser_socket_fd;
	AddressList peerList;
	AddressList serverList;
	fd_set active_fd_set, master_fd_set;
	int max_fd;
	char port[PORT_LEN];
	char hostname[HOSTNAME_LEN];
	char selfaddr[IP_LEN];
	Help * help;
	bool registered;
	char sync_filename[FILENAME_LEN];
public:
	Client();
	virtual ~Client();
	int clientInitialize(const char *);
	int clientServerConnect(const char * , const char *);
	int peerConnect(const char * , const char *);
	int checkFromServerList(const char *);
	int clientRun();
	command_code commandInterpretor(char *);
	int terminateConnection(int);
	int clientQuit();
	int receiveList();
	int requestFile(int,const char *);
	int sendFile(int,const char *);
	int putFile(int,const char *);
	int recvFile(int, const char *);
	int executeSync();
};

#endif
