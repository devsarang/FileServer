/*
 * Constants.h
 *
 *  Created on: Oct 6, 2015
 *      Author: sarang
 *
 *      It contains all the magic numbers, enums, and other hard codings required by the program
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_


enum command_code {
	UNKNOWN = -1, HELP, CREATOR, REGISTER, QUIT, DISPLAY,CONNECT,LIST,TERMINATE,PUT,GET,SYNC
};

#define MAX_CON 30
#define MAX_PEER 3
#define CMD_SIZE 3
#define CMD_LEN 100
#define ARG_LEN 100
#define PORT_LEN 6
#define BACKLOG 4
#define IP_LEN 16
#define HOSTNAME_LEN 40
#define INTERFACE_NAME1 "eth0"
#define INTERFACE_NAME2 "em2"
#define PEER_LIST_LEN 15
#define SOCK_LIST_LEN 15
#define FILENAME_LEN 50

#define BUFFER_SIZE 2400

#define MAX_PUT_DATA   1073741824     // 1 GB is the file limit for incoming PUT from peers

#endif
