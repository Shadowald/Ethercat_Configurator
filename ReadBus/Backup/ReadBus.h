#ifndef READBUS_H_INCLUDE
#define READBUS_H_INCLUDE

//#include <iostream>
//#include "ethercattype.h"
//#include "nicdrv.h"
//#include "ethercatmain.h"
//#include "ethercatconfig.h"
//#include "ethercatdc.h"
//#include "ethercatprint.h"
#include <boost/property_tree/ptree.hpp>
//#include <boost/property_tree/json_parser.hpp>

class readBus
{
public:
	int run();

private:
	bool startConfigEC(char* ifname);
	void checkStates();
	boost::property_tree::ptree createDevicesTree();
};
#endif