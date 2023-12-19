#ifndef READBUS_H_INCLUDE
#define READBUS_H_INCLUDE

#include <iostream>
#include "ethercattype.h"
#include "nicdrv.h"
#include "ethercatmain.h"
#include "ethercatconfig.h"
#include "ethercatdc.h"
#include "ethercatprint.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "EthercatWrapperInterface.h"

class ReadBus
{
	EthercatWrapperInterface* ethercatWrapper_ = 0;

public:
	//ReadBus() {}
	ReadBus(EthercatWrapperInterface* ethercatWrapper) :
		ethercatWrapper_(ethercatWrapper) {}
	~ReadBus() {}

	int run();

protected:
	int startConfigEC(char* ifname);
	int checkStates(ec_slavet* slaves, int slavecnt);
	void readStates();
	boost::property_tree::ptree createDevicesTree(ec_slavet* slaves, int slavecnt);
};
#endif