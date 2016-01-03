/*
 * sarangde_proj1.cpp
 *
 *  Created on: Oct 6, 2015
 *      Author: sarang
 *
 *      The main file which contains the main function and also creates the objects of the client and server classes.
 */

#include <iostream>
#include <cstdio>
#include <cstring>
#include "../inc/Help.h"
#include "../inc/Client.h"
#include "../inc/Server.h"
#include <cstdlib>
#include "../inc/Constants.h"

int main(int argc, char *argv[])
{
	char prog_mode;
	prog_mode = argv[1][0];
	char port[PORT_LEN];
	strcpy(port,argv[2]);
	if ( prog_mode == 'S' || prog_mode == 's')
	{

		Server *tcpServer = new Server();
		int ret = tcpServer->serverInitialize(port);
		if(ret != 0)
		{
			std::cout << "Server Initialization failed. Exiting the program.."<<std::endl;
			exit(1);
		}
		if( -1 == tcpServer->serverRun())
		{
			std::cout << "Server Exit during Run. Exiting .. " <<std::endl;
			exit(1);
		}
	}
	else if ( prog_mode == 'C' || prog_mode == 'c')
	{
		Client *tcpClient = new Client();
		int ret = tcpClient->clientInitialize(port);
		if(ret != 0)
		{
			std::cout << "Client Initialization failed. Exiting the program.."<<std::endl;
			exit(1);
		}
		if( -1 == tcpClient->clientRun())
		{
			std::cout << "Client Exit during Run. Exiting .. " << std::endl;
			exit(1);
		}
	}
	else
	{
		std::cout << "Wrong command line arguments provided"<<std::endl;
	}
	return 0;
}

