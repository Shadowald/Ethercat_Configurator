#pragma once
#include <stdint.h>

class EthercatWrapperInterface {
public:
	virtual ~EthercatWrapperInterface() {}

	virtual int ec_init(const char* ifname) = 0;
	virtual int ec_config(uint8_t usetable, void* pIOmap) = 0;
	virtual uint8_t ec_configdc() = 0;
	virtual uint16_t ec_statecheck(uint16_t slave, uint16_t reqstate, int timeout) = 0;
	virtual int ec_readstate() = 0;
	virtual char* ec_ALstatuscode2string(uint16_t ALstatuscode) = 0;
	virtual void ec_close() = 0;
};