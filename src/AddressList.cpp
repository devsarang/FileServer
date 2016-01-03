/*
 * AddressList.cpp
 *
 *  Created on: Oct 6, 2015
 *      Author: sarang
 *
 *      The AddressList class provides the list structure to store the address and other info. It provides deletion, addition, lookup and
 *      functions to store the object data in a stream and get the object back from a stream.
 */

#include "../inc/AddressList.h"
#include "../inc/Constants.h"
#include <iostream>
#include <cstring>
#include <iomanip>
#include <cstdlib>
#include <cstdio>

AddressList::AddressList()
{
	head = 0;
	iterator = 0;
}

AddressList::~AddressList()
{

}


void AddressList::printAddressList()
{
	Address * it = head;
	std::cout<<std::setw(2)<< "id" <<std::setw(18)<<"IP Address"<<std::setw(30)<< "Hostname " <<std::setw(10)<< "Port No" <<std::endl;
	while(it != 0)
	{
		std::cout<<std::setw(2)<< it->id <<std::setw(18)<<it->ipAddress<<std::setw(30)<< it->hostName<<std::setw(10)<< it->port_no <<std::endl;
		it = it->next;
	}
}

int AddressList::addAddress(int id,const char* Ip, const char* hostName,const char * port )
{
	Address* address = new Address();
	address->id = id;
	strcpy(address->hostName,hostName);
	strcpy(address->ipAddress,Ip);
	strcpy(address->port_no,port);
	address->next = NULL;
	address->next = 0;
	if (head == 0)
	{

		head = address;
		iterator = head;
	}
	else
	{
		iterator->next = address;
		iterator = iterator->next;
	}

	return 0;
}

int AddressList::deleteAddress(int id)
{
	Address * it = head;
	while(it->next != 0)
	{
		if(id == 1) //delete the head address
		{
			head = it->next;
			delete(it);
			return 0;
		}
		else if(it->next->id == id)
		{
			if(iterator == it->next)
				iterator = it;
			Address *temp = it->next;
			it->next = it->next->next;
			delete(temp);
			return 0;
		}
		else
			it = it->next;
	}
	return -1;
}

int AddressList::checkAddressExists(const char * host)
{
	Address * it = head;
	while (it != 0)
	{
		if(!strcmp(it->hostName,host)||!strcmp(it->ipAddress,host))
		{
			return 1;
		}
		it = it->next;
	}
	return 0;
}

int AddressList::checkIdExists(int id)
{
	Address * it = head;
	while (it != 0)
	{
		if(id == it->id)
		{
			return 1;
		}
		it = it->next;
	}
	return 0;
}
int AddressList::calcSize()
{
	Address * it = head;
	int count = 0;
	while (it != 0)
	{
		count++;
		it = it->next;
	}
	return count;
}

int AddressList::serialize(char * stream )
{
	Address * it = head;
	char * delim_obj = ":";
	char * delim_line = "@";
	strcpy(stream,"UPDATE ");
	while (it != 0)
	{
		char str_id[3];
		sprintf(str_id,"%d",it->id);
		strcat(stream,str_id);
		strcat(stream,delim_obj);
		strcat(stream,it->ipAddress);
		strcat(stream,delim_obj);
		strcat(stream,it->hostName);
		strcat(stream,delim_obj);
		strcat(stream,it->port_no);
		strcat(stream,delim_obj);
		strcat(stream,delim_line);
		it = it->next;
	}
	return 0;
}

int AddressList::clearAll()
{
	Address * it = head;
	while (it != 0)
	{
		Address *temp = it;
		it = it->next;
		delete(temp);
	}
	head = 0;
	return 0;
}

int AddressList::deSerialize(char * stream)
{
	if (head == 0)
	{
		char * delim_line = "@";
		char requestCmd[10];
		char argList[1024];
		strcpy(requestCmd, strtok(stream, " "));
		strcpy(argList, strtok(NULL, " "));
		if (!strcmp(requestCmd, "UPDATE"))
		{
			char* token = strtok(argList, delim_line);
			while (token)
			{
				int i=0;
				char id[3];
				char Ip[IP_LEN], hostName[HOSTNAME_LEN], port[PORT_LEN];

					int j = 0;
					while(token[i] != ':')
					{
						id[j++] = token[i++];
					}
					id[j]='\0';
					i++;
					j = 0;
					while(token[i] != ':')
					{
						Ip[j++] = token[i++];
					}
					Ip[j] = '\0';
					i++;
					j = 0;
					while(token[i] != ':')
					{
						hostName[j++] = token[i++];
					}
					hostName[j] = '\0';
					i++;
					j = 0;
					while(token[i] != ':')
					{
						port[j++] = token[i++];
					}
					port[j] = '\0';
				addAddress(atoi(id),Ip, hostName, port);
				token = strtok(NULL, delim_line);
			}
		}
		else
		{
			std::cout << "Wrong object for deserialization" << std::endl;
		}
	}
	else
	{
		std::cout<< "Cannot call this function on a already existing list. Clear it first"<< std::endl;
	}
	return 0;
}
