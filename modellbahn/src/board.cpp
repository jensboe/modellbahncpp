/*
 * Copyright (c) 2016-2017, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include "board.hpp"
#include <modm/architecture/interface/delay.hpp>
#include <modm/architecture/interface/accessor.hpp>
#include <modm/architecture/interface/assert.hpp>

Board::LoggerDevice loggerDevice;

// Set all four logger streams to use the UART
modm::log::Logger modm::log::debug(loggerDevice);
modm::log::Logger modm::log::info(loggerDevice);
modm::log::Logger modm::log::warning(loggerDevice);
modm::log::Logger modm::log::error(loggerDevice);

// Default all calls to printf to the UART
modm_extern_c void putchar_(char c)
{
	loggerDevice.write(c);
}

modm_extern_c void modm_abandon(const modm::AssertionInfo &info)
{
	MODM_LOG_ERROR << IFSS("Assertion '") << modm::accessor::asFlash(info.name) << IFSS("'");
	if (info.context != uintptr_t(-1))
	{
		MODM_LOG_ERROR << IFSS(" @ ") << (void *)info.context << IFSS(" (") << (uint32_t)info.context << IFSS(")");
	}
#if MODM_ASSERTION_INFO_HAS_DESCRIPTION
	MODM_LOG_ERROR << IFSS(" failed!\n  ") << modm::accessor::asFlash(info.description) << IFSS("\nAbandoning...\n");
#else
	MODM_LOG_ERROR << IFSS(" failed!\nAbandoning...\n");
#endif
	Board::Nucleo::Leds::setOutput();
	Board::Adapter_A::Indicator::Leds::setOutput();
	for (int times = 10; times >= 0; times--)
	{
		Board::Nucleo::LedRed::set(true);
		Board::Adapter_A::Indicator::LedRed::set(true);
		modm::delay_ms(20);
		Board::Nucleo::LedRed::set(false);
		Board::Adapter_A::Indicator::LedRed::set(false);
		modm::delay_ms(180);
	}
	// Do not flush here otherwise you may deadlock due to waiting on the UART
	// interrupt which may never be executed when abandoning in a higher
	// priority Interrupt!!!
	// MODM_LOG_ERROR << modm::flush;
}
