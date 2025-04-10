/*
 * Copyright (c) 2009, Martin Rosekeit
 * Copyright (c) 2009-2011, 2013, Fabian Greif
 * Copyright (c) 2012-2017, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#ifndef MODM_INTERFACE_PERIPHERAL_DRIVER_HPP
#define MODM_INTERFACE_PERIPHERAL_DRIVER_HPP

#include <stdint.h>
#include <cstddef>
#include <modm/math/tolerance.hpp>
#include <modm/math/units.hpp>

namespace modm
{

/**
 * Peripheral class
 *
 * This class acts as a base class for all classes describing the
 * public interface of common peripheral drivers.
 * As there is no implementation given, the classes specific to the platform
 * inherit from their respective base classes and must shadow the methods of
 * them.
 *
 * The inheritance is only visible for the documentation, it is completely
 * removed during compile time keeping a possible error at platform level.
 * This is safe, because only one platform can be compiled at once.
 *
 * This way, no virtual functions are needed and no overhead is generated,
 * but we still have clean inheritance in the documentation.
 * There is no need to document the platform specific implementation, since
 * it is taken from the base class.
 *
 * @ingroup modm_architecture
 * @author	Niklas Hauser
 */
class PeripheralDriver
{
#ifdef __DOXYGEN__
public:
	/// initializes the peripheral, must be called before use.
	static void
	initialize();

	/// configures a peripheral for a specific purpose
	static void
	configurePurpose();

	/// sets a parameter
	static void
	setParameter();

	/// returns a parameter
	static void
	getParameter();

	/// Read an interrupt flag.
	static bool
	getInterruptFlag();

	/**
	 * Acknowledge an interrupt flag.
	 *
	 * We use acknowledge here, since it describes the intention rather
	 * than the actual implementation.
	 */
	static void
	acknowledgeInterruptFlag();
#endif

public:
	/**
	 * Since baudrates are usually generated by prescaling a system clock,
	 * only several distinct values can be generated.
	 * This method checks if the user requested baudrate is within error
	 * tolerance of the system achievable baudrate.
	 */
	template< uint64_t available, uint64_t requested, percent_t tolerance >
	static void
	assertBaudrateInTolerance()
	{
		static_assert(modm::isValueInTolerance(requested, available, tolerance),
				"The closest available baudrate exceeds the tolerance of the requested baudrate!");
	}

	template< double available, double requested, percent_t tolerance >
	static void
	assertDurationInTolerance()
	{
		static_assert(modm::isValueInTolerance(requested, available, tolerance),
				"The closest available duration exceeds the tolerance of the requested duration!");
	}
};

} // namespace modm

#endif	// MODM_INTERFACE_PERIPHERAL_DRIVER_HPP
