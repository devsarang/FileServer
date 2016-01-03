/*
 * help.cpp
 *
 *  Created on: Sep 21, 2015
 *      Author: sarang
 *
 *      Definition of the help class
 */

#include "../inc/Help.h"

#include <iostream>
using namespace std;

Help::Help()
{
	error_code = 0;
}

Help::~Help()
{}



void Help::displayUsageClient()
{
	cout<< "Following commands are available:"<<endl;
	cout<< "help : Display the command usage"<<endl;
	cout<< "creator : Displays the name of the creator"<<endl;
	cout<< "display : Displays the IP address and HOSTNAME of the process"<<endl;
	cout<< "register : Connects to the Server specified. Usage : register <server IP/HOSTNAME> <PORT>"<<endl;
	cout<< "connect : COnnects to a peer specified. Usage : connect <client IP/HOSTNAME> <PORT>"<<endl;
	cout<< "list : Displays list of connections"<<endl;
}

void Help::displayUsageServer()
{
	cout<< "Following commands are available:"<<endl;
	cout<< "help : Display the command usage"<<endl;
	cout<< "creator : Displays the name of the creator"<<endl;
	cout<< "display : Displays the IP address and HOSTNAME of the process"<<endl;
	cout<< "list : Displays list of connections"<<endl;
}

