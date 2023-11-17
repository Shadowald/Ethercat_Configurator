/** \file
 * \brief json file writer for EtherCAT master
 *
 * Writes all adapter names and relavant device data, if devices exist, to a json file for later use in a UI.
 * Was made by modifying example code for Simple Open EtherCAT master.
 *
 * Forest Feltner 2023
 */

#include <iostream>
#include "ethercattype.h"
#include "nicdrv.h"
#include "ethercatmain.h"
#include "ethercatconfig.h"
#include "ethercatdc.h"
#include "ethercatprint.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


bool startConfigEC(char* ifname)
{
    char IOmap[4096];
    int expectedWKC;

    printf("Initializing %s\n", ifname);

    /* initialise adapter, bind socket to ifname */
    if (ec_init(ifname))
    {
        printf("ec_init on %s succeeded.\n", ifname);

        /* find and auto-config devices */
        if (ec_config(false, &IOmap) > 0)
        {
            printf("Configuring devices\n");
            ec_configdc();

            while (EcatError) printf("%s", ec_elist2string());
            printf("%d devices found and configured.\n", ec_slavecount);

            expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
            printf("Calculated workcounter %d\n", expectedWKC);

            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        printf("No socket connection on %s\nExcecute as root\n", ifname);
        return false;
    }
}

void checkStates()
{
    /* wait for all devices to reach SAFE_OP state */
    ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 3);
    if (ec_slave[0].state != EC_STATE_SAFE_OP)
    {
        printf("Not all devices reached safe operational state.\n");
        ec_readstate();
        for (int i = 1; i <= ec_slavecount; i++)
        {
            if (ec_slave[i].state != EC_STATE_SAFE_OP)
            {
                printf("Device %d State=%2x StatusCode=%4x : %s\n",
                    i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
            }
        }
    }
}

void readStates()
{
    ec_readstate();
    /* displays relevant data for each device*/
    for (int cnt = 1; cnt <= ec_slavecount; cnt++)
    {
        printf("\nDevice:%d\n Name:%s\n Output size: %dbits\n Input size: %dbits\n",
            cnt, ec_slave[cnt].name, ec_slave[cnt].Obits, ec_slave[cnt].Ibits);
       
        printf(" Configured address: %4.4x\n Alias address: %4.4x\n Index: %4.4x\n", 
            ec_slave[cnt].configadr, ec_slave[cnt].aliasadr, ec_slave[cnt].configindex);
       
        printf(" Man: %8.8x ID: %8.8x \n", (int)ec_slave[cnt].eep_man, (int)ec_slave[cnt].eep_id);        
    }
}

boost::property_tree::ptree createDevicesTree()
{
    boost::property_tree::ptree tree;
    boost::property_tree::ptree devices;

    printf("Populating property tree for devices\n");
    for (int cnt = 1; cnt <= ec_slavecount; cnt++)
    {
        std::string device = "device" + std::to_string(cnt);
        boost::property_tree::ptree child;

        /* Relevant device info */
        child.put("name", ec_slave[cnt].name);
        child.put("eep.man", ec_slave[cnt].eep_man);
        child.put("eep.id", ec_slave[cnt].eep_id);
        child.put("address.configured", ec_slave[cnt].configadr);
        child.put("address.alias", ec_slave[cnt].aliasadr);
        child.put("index", ec_slave[cnt].configindex);
        child.put("bits.output", ec_slave[cnt].Obits);
        child.put("bits.input", ec_slave[cnt].Ibits);

        devices.push_back(std::make_pair("", child));
    }
    printf("Property tree for devices is populated\n");

    return devices;
}

int run()
{
    ec_adaptert* adapter = NULL;

    printf("read Bus and write relevant information to json file\n");
    /* Print the list */
    printf("Finding adapters\n");
    adapter = ec_find_adapters();

    boost::property_tree::ptree root;
    boost::property_tree::ptree children;
    

    printf("Populating property tree\n");

    while (adapter != NULL)
    {
        boost::property_tree::ptree child;
        boost::property_tree::ptree devices;

        child.put("name", adapter->name);

        if (startConfigEC(adapter->name))
        {
            checkStates();
            readStates();
            devices = createDevicesTree();
            child.add_child("devices", devices);
        }
        children.push_back(std::make_pair("", child));
        
        adapter = adapter->next;
    }
    ec_close();

    root.add_child("adapters", children);
    printf("Writing property tree to json file\n");
    boost::property_tree::write_json("ethercatBus.json", root);

    printf("End program\n");
    return (0);
}