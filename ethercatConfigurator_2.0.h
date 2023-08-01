#pragma once

#include <stdio.h>
#include <string>
#include <map>
#include <vector>

class CEthercatConfigurator
{
public:
    // The different device types that exist within ethercat adapter
    enum class EthercatDeviceType
    {
        NotDefined,
        Chutes,
        FlapsLamps,
        Scales,
        Inverter,
    };

    // SHould this be 2 different structs?
    // Holds all neccessarry information needed from devices on ethercat adapter
    struct DEVICE {
        std::string name;
        int eepMan;
        int eepId;
        int addressConfig;
        int addressAlias;
        int index;
        int Obits;
        int Ibits;
        EthercatDeviceType deviceType = EthercatDeviceType::NotDefined;
        int firstChute = 0;
        int lastChute = 0;
    };

    std::string ifbuf;//[1024]; //Get rid of this later, only used in legacy function, not actual program
    //std::map<std::string, bool> assignChutesForward;
    std::map<std::string, std::vector<DEVICE>> adapterDevices;
    //std::map<std::string, std::vector<DEVICE>> devicesFromFile;
    std::vector<std::string> allAdapters;
    std::vector<DEVICE> currentDevices;
    std::string currentAdapterName;
    bool adapterChutesDirection = true;

    CEthercatConfigurator();
    virtual ~CEthercatConfigurator();

    void runEthercatConfigurator();
    void init();
    void init(std::string filename);
    void writeSettings(std::string pwd);
    bool loadSettings(std::string filename);
    void transferSettings(std::string ifname);

private:
    void writeBackup(std::string pwd);
};