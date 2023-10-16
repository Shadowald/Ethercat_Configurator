// ethercatConfigurator_2.0.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
/** \file
 * \brief Configures ethercat devices according to the user and writes them to a json file to be used by another program. 
 *
 * Loads in a json file that read the Bus, allowing the user to select ethercat adapter to view (only those with slaves are shown) and configure.
 * Once configured, writes all neccesarry info to a new json file. Program can also compare currently selected adapter configuration with adapter configuration
 * already written to existing json file.
 *
 * (c)Forest Feltner 2023
 */

#include <stdio.h>
#include <string.h>
#include <map>
#include "ethercatConfigurator_2.0.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <windows.h>

CEthercatConfigurator::CEthercatConfigurator()
{
    init();
}

CEthercatConfigurator::~CEthercatConfigurator()
{
}

// Writes a backup file before rewriting a new .json file
// Currently only reads in a file from set path and does not check if said file exists
void CEthercatConfigurator::writeBackup(std::string pwd)
{
    boost::property_tree::ptree root;

    // This try..catch prevents failure when no file previously exists to create a backup from
    try
    {
        // Test: see that file was read and new file was written
        boost::property_tree::read_json(pwd + "\\ethercat.json", root);
        boost::property_tree::write_json(pwd + "\\ethercatBackup.json", root);
    }
    catch (const std::exception e)
    {
        return;
    }
}

void CEthercatConfigurator::writeSettings(std::string pwd)
{
    writeBackup(pwd);

    // Code was designed this way so that root of the tree is the "adapter" and all related info is held underneath it, ie name, Chutes.AssignForward, and devices
    boost::property_tree::ptree root;
    boost::property_tree::ptree children;
    boost::property_tree::ptree devices;

    children.put("name", currentAdapterName);
    children.put("Chutes.AssignForward", adapterChutesDirection);
    for (int i = 0; i < currentDevices.size(); i++)
    {
        boost::property_tree::ptree child;
        int deviceType;

        //Device info
        child.put("name", currentDevices[i].name);
        child.put("eep.man", currentDevices[i].eepMan);
        child.put("eep.id", currentDevices[i].eepId);
        child.put("address.configured", currentDevices[i].addressConfig);
        child.put("address.alias", currentDevices[i].addressAlias);
        child.put("index", currentDevices[i].index);
        child.put("bits.output", currentDevices[i].Obits);
        child.put("bits.input", currentDevices[i].Ibits);
        deviceType = static_cast<int>(currentDevices[i].deviceType);
        child.put("type", deviceType); 
        child.put("chute.first", currentDevices[i].firstChute);
        child.put("chute.last", currentDevices[i].lastChute);

        // This ensures that all devices are written into the json file as an array under "adapter"
        devices.push_back(std::make_pair("", child));
    }

    children.add_child("devices", devices);
    root.add_child("adapter", children);

    //Write settings to file in set location given by user
    //TODO: get and set proper destination path from user, then reimplement the oringial write function calls
    // 
    boost::property_tree::write_json(pwd + "\\ethercat.json", root);
    //boost::property_tree::write_json((DestinationPath + "/ethercat.json").toStdString(), tree,);   
}

void CEthercatConfigurator::init()
{
    // Test: Check that these are cleared
    allAdapters.clear();
    adapterDevices.clear();
    adapterDevices.clear();

    // Test: Check that program is executed
    const HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    ShellExecute(NULL, L"open", L"ReadBus.exe", NULL, NULL, SW_SHOW);

    boost::property_tree::ptree root;
    try
    {
        // Test: See that file was read
        boost::property_tree::read_json("ethercatBus.json", root);
    }
    catch(const std::exception e)
    {
        return;
    }

    // Populates adapterDevices and assignChutesForward maps from json file    
    for (boost::property_tree::ptree::value_type& adapter : root.get_child("adapters"))
    {
        std::vector<DEVICE> devices;
        allAdapters.push_back(adapter.second.get<std::string>("name"));

        // Test: See that devices exist
        // Only adapters that have devices are added to the maps, with the adapter name being the key
        try
        {
            for (boost::property_tree::ptree::value_type& device_pt : adapter.second.get_child("devices"))
            {
                DEVICE device_st;
                device_st.name = device_pt.second.get<std::string>("name");
                device_st.eepMan = device_pt.second.get<int>("eep.man");
                device_st.eepId = device_pt.second.get<int>("eep.id");
                device_st.addressConfig = device_pt.second.get<int>("address.configured");
                device_st.addressAlias = device_pt.second.get<int>("address.alias");
                device_st.index = device_pt.second.get<int>("index");
                device_st.Obits = device_pt.second.get<int>("bits.output");
                device_st.Ibits = device_pt.second.get<int>("bits.input");
                devices.push_back(device_st);
            }
            adapterDevices[adapter.second.get<std::string>("name")] = devices;
            //assignChutesForward[adapter.second.get<std::string>("name")] = true;
        }

        catch (const std::exception e)
        {
            // Test: See that no devices exist
            std::string tempSt = adapter.second.get<std::string>("name");
            printf("There are no devices connected to adapter %s.\nMoving on to next adapter now.\n", (&tempSt)->c_str());
            //const char* temp = adapter.second.get<const char*>("name");
        }
    }
}

// Same tests for other init?
void CEthercatConfigurator::init(std::string filename)
{
    allAdapters.clear();
    adapterDevices.clear();

    boost::property_tree::ptree root;

    boost::property_tree::read_json(filename, root);

    try
    {    
        // Populates adapterDevices and assignChutesForward maps from json file    
        for (boost::property_tree::ptree::value_type& adapter : root.get_child("adapters"))
        {
            std::vector<DEVICE> devices;
            allAdapters.push_back(adapter.second.get<std::string>("name"));

            // Only adapters that have devices are added to the maps, with the adapter name being the key
            try
            {
                for (boost::property_tree::ptree::value_type& device_pt : adapter.second.get_child("devices"))
                {
                    DEVICE device_st;
                    device_st.name = device_pt.second.get<std::string>("name");
                    device_st.eepMan = device_pt.second.get<int>("eep.man");
                    device_st.eepId = device_pt.second.get<int>("eep.id");
                    device_st.addressConfig = device_pt.second.get<int>("address.configured");
                    device_st.addressAlias = device_pt.second.get<int>("address.alias");
                    device_st.index = device_pt.second.get<int>("index");
                    device_st.Obits = device_pt.second.get<int>("bits.output");
                    device_st.Ibits = device_pt.second.get<int>("bits.input");
                    devices.push_back(device_st);
                }
                adapterDevices[adapter.second.get<std::string>("name")] = devices;
                //assignChutesForward[adapter.second.get<std::string>("name")] = true;
            }

            catch (const std::exception e)
            {
                std::string tempSt = adapter.second.get<std::string>("name");
                printf("There are no devices connected to adapter %s.\nMoving on to next adapter now.\n", (&tempSt)->c_str());
            }
        }
    }
    catch (const std::exception e)
    {
        printf("\n%s does not contain the correct information.\nPlease select a file that has the correct information.\n", (&filename)->c_str());
    }
}

bool CEthercatConfigurator::loadSettings(std::string filename)
{
    // Get adapter configuration from file for comparison  
    boost::property_tree::ptree root;
    std::vector<DEVICE> devices;

    // Test: See that file was read
    boost::property_tree::read_json(filename, root);

    // Try..catch used to prevent failure if incorrect information is in loaded json file
    try
    {
        // Test: See that values are correct
        currentAdapterName = root.get<std::string>("adapter.name");
        adapterChutesDirection = root.get<bool>("adapter.Chutes.AssignForward");
        
        // Test: See that devices exist
        // Populates devicesFromFile with the devices written in the json file
        for (boost::property_tree::ptree::value_type& device_pt : root.get_child("adapter.devices"))
        {
            DEVICE device_st;
            EthercatDeviceType deviceType;
            device_st.name = device_pt.second.get<std::string>("name");
            device_st.eepMan = device_pt.second.get<int>("eep.man");
            device_st.eepId = device_pt.second.get<int>("eep.id");
            device_st.addressConfig = device_pt.second.get<int>("address.configured");
            device_st.addressAlias = device_pt.second.get<int>("address.alias");
            device_st.index = device_pt.second.get<int>("index");
            device_st.Obits = device_pt.second.get<int>("bits.output");
            device_st.Ibits = device_pt.second.get<int>("bits.input");
            // TODO: Determine how to handle the deviceType information, ie what data type to use
            deviceType = static_cast<EthercatDeviceType>(device_pt.second.get<int>("type"));
            device_st.deviceType = deviceType;
            device_st.firstChute = device_pt.second.get<int>("chute.first");
            device_st.lastChute = device_pt.second.get<int>("chute.last");
            devices.push_back(device_st);
        }

        currentDevices = devices;

        return true;
    }
    catch (const std::exception e)
    {
        printf("\n%s does not contain the correct information.\nPlease select a file that has the correct information.\n", (&filename)->c_str());
        return false;
    }
}

void CEthercatConfigurator::transferSettings(std::string ifname)
{
    // Moves inofrmation from display table to the write-to-file table
    std::vector<DEVICE> devices;
    currentAdapterName = ifname;
    for(int i = 0; i < adapterDevices[ifname].size(); i++)
    {
        DEVICE device_st;
        device_st.name = adapterDevices[ifname][i].name;
        device_st.eepMan = adapterDevices[ifname][i].eepMan;
        device_st.eepId = adapterDevices[ifname][i].eepId;
        device_st.addressConfig = adapterDevices[ifname][i].addressConfig;
        device_st.addressAlias = adapterDevices[ifname][i].addressAlias;
        device_st.index = adapterDevices[ifname][i].index;
        device_st.Obits = adapterDevices[ifname][i].Obits;
        device_st.Ibits = adapterDevices[ifname][i].Ibits;
        if (currentDevices.size())
        {
            device_st.deviceType = currentDevices[i].deviceType;
            device_st.firstChute = currentDevices[i].firstChute;
            device_st.lastChute = currentDevices[i].lastChute;
        }        
        devices.push_back(device_st);
    }

    currentDevices = devices;
}

// Get rid of this function at some point; just legacy code that was used to test back-end functions before UI was made
void CEthercatConfigurator::runEthercatConfigurator()
{    
    printf("\nEthercat Configurator 2.0\n");
    printf("init started\n");
    init();
    printf("init finished\n");

    // If adapters have devices, the adapters names are displayd to be selected, unless there is only one adapter in which case it is automatically displayed
    // If no adapters have devices, then the program terminates
    /* Reinstate once GUI is ready*/
    if (adapterDevices.size() == 1)
    {
        // start slaveinfo
        auto name = adapterDevices.begin();
        std::string temp = name->first;
        printf("\nOnly one adapter found. Loading and writing settings now.\n");
        //strcpy_s(ifbuf, (unsigned)_countof(ifbuf), adapters[0]);
        //writeSettings();
    }
    else if (adapterDevices.size())
    {
        printf("\nPlease select a device name to use by name\n");
        for (auto i = adapterDevices.begin(); i != adapterDevices.end(); i++)
        {
            std::string temp = i->first;
            printf("%s\n", (&temp)->c_str());
        }            
        //scanf_s("%s", ifbuf, 1024);
        std::cin >> ifbuf;
    
        /* start slaveinfo */
        //strcpy_s(ifbuf, (unsigned)_countof(ifbuf), adapters[deviceNum]);
        //writeSettings();
        printf("json file written\n");
       
    }
    else
    {
        printf("\nThere are no devices with devices to configure.\nProgram is terminating now.\n");
    }

    // Configuration of an already saved adapter is loaded and compared to currently selected adapter
    // TODO: make this a user decision and not done automatically when UI is established
    //loadSettings();
    transferSettings(ifbuf);
    printf("loop restarts\n");
    

    printf("End program\n");
    return;
}