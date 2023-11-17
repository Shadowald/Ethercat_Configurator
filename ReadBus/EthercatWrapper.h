#pragma once

#include "ethercattype.h"
#include "nicdrv.h"
#include "ethercatmain.h"
#include "ethercatconfig.h"
#include "ethercatdc.h"
#include "ethercatprint.h"

#include <stdint.h>
#include "EthercatWrapperInterface.h"

class EthercatWrapper : public EthercatWrapperInterface {
public:
	~EthercatWrapper() {}

	int ec_init(const char* ifname) override {
		return ::ec_init(ifname);
	}
	int ec_config(uint8_t usetable, void* pIOmap) override {
		return ::ec_config(usetable, pIOmap);
	}
	uint16_t ec_statecheck(uint16_t slave, uint16_t reqstate, int timeout) override {
		return ::ec_statecheck(slave, reqstate, timeout);
	}
	int ec_readstate() override {
		return ::ec_readstate();
	}
	void ec_close() override {
		return ::ec_close();
	}
};

//virtual int ec_init(const char* ifname) = 0;
//virtual int ec_config(uint8_t usetable, void* pIOmap) = 0;
//virtual uint16_t ec_statecheck(uint16_t slave, uint16_t reqstate, int timeout) = 0;
//virtual int ec_readstate(void) = 0;
//virtual void ec_close(void) = 0;