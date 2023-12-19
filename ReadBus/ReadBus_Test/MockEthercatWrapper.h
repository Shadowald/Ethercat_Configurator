#pragma once

#include <stdint.h>
#include "EthercatWrapperInterface.h"
#include "gmock/gmock.h"

class MockEthercatWrapper : public EthercatWrapperInterface {
public:
	~MockEthercatWrapper() {}

	MOCK_METHOD(int, ec_init, (const char*), (override));
	MOCK_METHOD(int, ec_config, (uint8_t, void*), (override));
	MOCK_METHOD(uint8_t, ec_configdc, (), (override));
	MOCK_METHOD(uint16_t, ec_statecheck, (uint16_t, uint16_t, int), (override));
	MOCK_METHOD(int, ec_readstate, (), (override));
	MOCK_METHOD(char*, ec_ALstatuscode2string, (uint16_t), (override));
	MOCK_METHOD(void, ec_close, (), (override));
};

//virtual int ec_init(const char* ifname) = 0;
//virtual int ec_config(uint8_t usetable, void* pIOmap) = 0;
//virtual uint16_t ec_statecheck(uint16_t slave, uint16_t reqstate, int timeout) = 0;
//virtual int ec_readstate(void) = 0;
//virtual void ec_close(void) = 0;