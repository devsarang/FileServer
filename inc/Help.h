/*
 * help.h
 *
 *  Created on: Sep 21, 2015
 *      Author: sarang
 *
 *      Declaration of the Help Class.
 */


#ifndef HELP_H_
#define HELP_H_

class Help
{

	int error_code;

public:

	Help();
	virtual ~Help();
	void displayUsageServer();
	void displayUsageClient();
};

#endif /* HELP_H_ */
