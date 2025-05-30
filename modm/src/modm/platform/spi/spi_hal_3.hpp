/*
 * Copyright (c) 2013, Kevin Läufer
 * Copyright (c) 2013-2018, Niklas Hauser
 * Copyright (c) 2014, Daniel Krebs
 * Copyright (c) 2020, Mike Wolfram
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#ifndef MODM_STM32_SPI_HAL3_HPP
#define MODM_STM32_SPI_HAL3_HPP

#include "spi_base.hpp"

namespace modm
{

namespace platform
{

/**
 * Serial peripheral interface (SPI3)
 *
 * Very basic implementation that exposes more hardware features than
 * the regular Spi classes.
 *
 * @author		Kevin Laeufer
 * @ingroup		modm_platform_spi modm_platform_spi_3
 */
class SpiHal3 : public SpiBase
{
public:
	/// Enables the clock, resets the hardware and sets the SPE bit
	static void
	enable();

	/// Disables the hw module (by disabling its clock line)
	static void
	disable();

	/**
	 * Initialize Spi Peripheral
	 *
	 * Enables clocks, the UART peripheral
	 * Sets baudrate and parity.
	 */
	static void
	initialize(	Prescaler prescaler,
				MasterSelection masterSelection = MasterSelection::Master,
				DataMode dataMode   = DataMode::Mode0,
				DataOrder dataOrder = DataOrder::MsbFirst,
				DataSize  dataSize  = DataSize::Bit8);

	static void
	setDataMode(DataMode dataMode);

	static void
	setDataOrder(DataOrder dataOrder);

	static void
	setDataSize(DataSize dataSize);

	static void
	setMasterSelection(MasterSelection masterSelection);

	/// Returns true if data has been received
	static bool
	isReceiveRegisterNotEmpty();

	/// Returns true if data can be written
	static bool
	isTransmitRegisterEmpty();

	/**
	 * Write up to 16 Bit to the data register
	 *
	 * @warning 	This method does NOT do any sanity checks!!
	 *				It is your responsibility to check if the register
	 *				is empty!
	 */
	static void
	write(uint16_t data);

	/**
	 * Write 8 Bit to the data register
	 *
	 * @warning 	This method does NOT do any sanity checks!!
	 *				It is your responsibility to check if the register
	 *				is empty!
	 */
	static void
	write(uint8_t data);

	/**
	 * Returns the value of the data register
	 *
	 * @warning 	This method does NOT do any sanity checks!!
	 *				It is your responsibility to check if the register
	 *				contains something useful!
	 */
	static void
	read(uint8_t &data);

	/**
	 * Returns the value of the data register
	 *
	 * @warning 	This method does NOT do any sanity checks!!
	 *				It is your responsibility to check if the register
	 *				contains something useful!
	 */
	static void
	read(uint16_t &data);

	static void
	enableInterruptVector(bool enable, uint32_t priority);

	static void
	enableInterrupt(Interrupt_t interrupt);

	static void
	disableInterrupt(Interrupt_t interrupt);

	static InterruptFlag_t
	getInterruptFlags();

	/**
	 *
	 *
	 * @warning 	Not all InterruptFlags can be cleared this way.
	 */
	static void
	acknowledgeInterruptFlag(InterruptFlag_t flags);

	static void
	enableTransfer();

	static void
	disableTransfer();
};

} // namespace platform

} // namespace modm

#include "spi_hal_3_impl.hpp"

#endif // MODM_STM32_SPI_HAL3_HPP