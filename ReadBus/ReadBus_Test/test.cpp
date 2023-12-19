//#include "pch.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "MockEthercatWrapper.h"
#include "ReadBus.h"

using ::testing::Return;
using ::testing::_;

//typedef struct ec_slave
//{
//    /** state of slave */
//    uint16           state;
//    /** Configured address */
//    uint16           configadr;
//    /** Alias address */
//    uint16           aliasadr;
//    /** Manufacturer from EEprom */
//    uint32           eep_man;
//    /** ID from EEprom */
//    uint32           eep_id;
//    /** output bits */
//    uint16           Obits;
//    /** input bits */
//    uint16           Ibits;
//    /** link to config table */
//    uint16           configindex;
//    /** readable name */
//    char             name[EC_MAXNAME + 1];
//} ec_slavet;

// Wrapper used to access and test protected functions
class ReadBusTestClass : public ReadBus {

public:
    ReadBusTestClass(EthercatWrapperInterface* ethercatWrapper) :
        ReadBus(ethercatWrapper) {}
    ~ReadBusTestClass() {}

    int wrapStartConfigEC(char* ifname) { return startConfigEC(ifname); }
    int wrapCheckStates(ec_slavet* slaves, int slavecnt) { return checkStates(slaves, slavecnt); }
    void wrapReadStates() { return readStates(); }
    boost::property_tree::ptree wrapCreateDevicesTree(ec_slavet* slaves, int slavecnt) { createDevicesTree(slaves, slavecnt); }
};

TEST(ReadBusTest, StartConfigECTest) {

    ec_adaptert* adapter;
    adapter = ec_find_adapters();

    // Used a variable for this string, because test would fail if the address wasn't the same
    // Might be due to function taking a popinter as a parameter
    char* failedConnection = "string";
    
    MockEthercatWrapper mock_ethercatwrapper;
    EXPECT_CALL(mock_ethercatwrapper, ec_init(failedConnection))
        .Times(1)
        .WillOnce(Return(0));           // 1st Test
    EXPECT_CALL(mock_ethercatwrapper, ec_init(adapter->name))
        .Times(2)
        .WillRepeatedly(Return(1));     // 2nd and 3rd Test

    // _ is used to match with any input used for function call
    EXPECT_CALL(mock_ethercatwrapper, ec_config(_, _))
        .Times(2)
        .WillOnce(Return(-1))           // 2nd Test
        .WillOnce(Return(2));           // 3rd Test
    
    ReadBusTestClass reader(&mock_ethercatwrapper);
    EXPECT_EQ(2, reader.wrapStartConfigEC(failedConnection));
    EXPECT_EQ(1, reader.wrapStartConfigEC(adapter->name));
    EXPECT_EQ(0, reader.wrapStartConfigEC(adapter->name));
}

//TEST(ReadBusTest, CheckStatesTest) {
//    // With MockG use adapter with device that wont reach safe OP
//    EXPECT_EQ(1, checkStates());
//    // With MockG use adapter with device that will reach safe OP
//    EXPECT_EQ(0, checkStates());
//}
//
//TEST(ReadBusTest, CreateDeviceTreeTest) {
//    // With MockG use adapter with set number of devices
//    EXPECT_EQ(1, createDevicesTree().size());
//    EXPECT_NE(0, createDevicesTree().size());
//    boost::property_tree::ptree devices;
//    devices = createDevicesTree();
//    // Test that devices have been correctly saved, or use MockG and check coded strings from that
//    //EXPECT_STREQ(ec_slave[0].name, devices[0].name);
//}

// Should we have a test to confirm that the json file is written?
// If so how would we test for that?

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}