/*
 * AddressList.h

 *
 *  Created on: Oct 6, 2015
 *      Author: sarang
 *
 *      Declaration for the AddresList Class Address structure
 */

#ifndef INC_ADDRESSLIST_H_
#define INC_ADDRESSLIST_H_

#include "Constants.h"

class AddressList {

	struct Address
	{
		int id;
		char hostName[HOSTNAME_LEN];
		char ipAddress[IP_LEN];
		char port_no[PORT_LEN];
		Address * next;
	};
	struct Address * head;
	struct Address * iterator;
public:
	AddressList();
	virtual ~AddressList();
	void printAddressList();
	int addAddress(int,const char *, const char *,const char *);
	int checkAddressExists(const char *);
	int deleteAddress(int);
	int calcSize();
	int checkIdExists(int);
	int serialize(char *);
	int deSerialize(char *);
	int clearAll();
};

#endif /* INC_ADDRESSLIST_H_ */
