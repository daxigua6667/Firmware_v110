/****************************************************************************
 *
 *   Copyright (c) 2019 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file bmp388_spi.cpp
 *
 * SPI interface for BMP388 (NOTE: untested!)
 */

#include <drivers/device/spi.h>

#include "bmp388.h"


/* SPI protocol address bits */
#define DIR_READ			(1<<7)  //for set
#define DIR_WRITE			~(1<<7) //for clear

#if defined(PX4_SPIDEV_BARO) || defined(PX4_SPIDEV_EXT_BARO)

#pragma pack(push,1)
struct spi_data_s {
	uint8_t addr;
	struct data_s data;
};

struct spi_calibration_s {
	uint8_t addr;
	struct calibration_s cal;
};
#pragma pack(pop)

class BMP388_SPI: public device::SPI, public IBMP388
{
public:
	BMP388_SPI(uint8_t bus, uint32_t device, bool is_external_device);
	virtual ~BMP388_SPI() = default;

	bool is_external();
	int init();

	uint8_t get_reg(uint8_t addr);
	int get_reg_buf(uint8_t addr, uint8_t *buf, uint8_t len);
	int set_reg(uint8_t value, uint8_t addr);
	data_s *get_data(uint8_t addr);
	calibration_s *get_calibration(uint8_t addr);

	uint32_t get_device_id() const override { return device::SPI::get_device_id(); }

private:
	spi_calibration_s _cal;
	spi_data_s _data;
	bool _external;
};

IBMP388 *bmp388_spi_interface(uint8_t busnum, uint32_t device, bool external)
{
	return new BMP388_SPI(busnum, device, external);
}

BMP388_SPI::BMP388_SPI(uint8_t bus, uint32_t device, bool is_external_device) :
	SPI("BMP388_SPI", nullptr, bus, device, SPIDEV_MODE3, 10 * 1000 * 1000)
{
	_external = is_external_device;
}

bool BMP388_SPI::is_external()
{
	return _external;
};

int BMP388_SPI::init()
{
	return SPI::init();
};

uint8_t BMP388_SPI::get_reg(uint8_t addr)
{
	uint8_t cmd[2] = { (uint8_t)(addr | DIR_READ), 0}; //set MSB bit
	transfer(&cmd[0], &cmd[0], 2);

	return cmd[1];
}

int BMP388_SPI::get_reg_buf(uint8_t addr, uint8_t *buf, uint8_t len)
{
	uint8_t cmd[1] = {(uint8_t)(addr | DIR_READ)};
	return transfer(&cmd[0], buf, len);
}

int BMP388_SPI::set_reg(uint8_t value, uint8_t addr)
{
	uint8_t cmd[2] = { (uint8_t)(addr & DIR_WRITE), value}; //clear MSB bit
	return transfer(&cmd[0], nullptr, 2);
}

data_s *BMP388_SPI::get_data(uint8_t addr)
{
	_data.addr = (uint8_t)(addr | DIR_READ); //set MSB bit

	if (transfer((uint8_t *)&_data, (uint8_t *)&_data, sizeof(struct spi_data_s)) == OK) {
		return &(_data.data);

	} else {
		return nullptr;
	}
}

calibration_s *BMP388_SPI::get_calibration(uint8_t addr)
{
	_cal.addr = addr | DIR_READ;

	if (transfer((uint8_t *)&_cal, (uint8_t *)&_cal, sizeof(struct spi_calibration_s)) == OK) {
		return &(_cal.cal);

	} else {
		return nullptr;
	}
}

#endif /* PX4_SPIDEV_BARO || PX4_SPIDEV_EXT_BARO */
