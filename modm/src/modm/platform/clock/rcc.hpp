/*
 * Copyright (c) 2012, 2016, Sascha Schade
 * Copyright (c) 2012, 2017, Fabian Greif
 * Copyright (c) 2012, 2014-2017, Niklas Hauser
 * Copyright (c) 2013-2014, Kevin Läufer
 * Copyright (c) 2018, 2021-2022, Christopher Durand
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#ifndef MODM_STM32_RCC_HPP
#define MODM_STM32_RCC_HPP

#include <cstdint>
#include "../device.hpp"
#include <modm/platform/core/peripherals.hpp>
#include <modm/platform/gpio/connector.hpp>
#include <modm/architecture/interface/delay.hpp>

namespace modm::platform
{

/**
 * Reset and Clock Control for STM32 devices.
 *
 * This class abstracts access to clock settings on the STM32.
 * You need to use this class to enable internal and external clock
 * sources & outputs, set PLL parameters and AHB & APB prescalers.
 * Don't forget to set the flash latencies.
 *
 * @author		Niklas Hauser
 * @ingroup		modm_platform_rcc
 */
class Rcc
{
public:
	static constexpr uint32_t LsiFrequency = 32'000;
	static constexpr uint32_t HsiFrequency = 16'000'000;
	static constexpr uint32_t BootFrequency = 16'000'000;

	enum class
	PllSource : uint32_t
	{
		/// High speed internal clock (16 MHz)
		Hsi = RCC_PLLCFGR_PLLSRC_HSI,
		InternalClock = Hsi,
		/// High speed external clock
		Hse = RCC_PLLCFGR_PLLSRC_HSE,
		ExternalClock = Hse,
		ExternalCrystal = Hse,
	};
	enum class
	SystemClockSource : uint32_t
	{
		Hsi = RCC_CFGR_SW_HSI,
		Hse = RCC_CFGR_SW_HSE,
		InternalClock = Hsi,
		ExternalClock = Hse,
		ExternalCrystal = Hse,
		Pll = RCC_CFGR_SW_PLL,
	};

	enum class
	RealTimeClockSource : uint32_t
	{
		Lsi = RCC_BDCR_RTCSEL_1,
		Lse = RCC_BDCR_RTCSEL_0,
		Hse = RCC_BDCR_RTCSEL_0 | RCC_BDCR_RTCSEL_1,

		ExternalClock = Hse,
		ExternalCrystal = Hse,
		LowSpeedInternalClock = Lsi,
		LowSpeedExternalClock = Lse,
		LowSpeedExternalCrystal = Lse
	};

	enum class
	WatchdogClockSource : uint32_t
	{
		LowSpeedInternalClock = 0
	};

	enum class
	AhbPrescaler : uint32_t
	{
		Div1   = RCC_CFGR_HPRE_DIV1,
		Div2   = RCC_CFGR_HPRE_DIV2,
		Div4   = RCC_CFGR_HPRE_DIV4,
		Div8   = RCC_CFGR_HPRE_DIV8,
		Div16  = RCC_CFGR_HPRE_DIV16,
		Div64  = RCC_CFGR_HPRE_DIV64,
		Div128 = RCC_CFGR_HPRE_DIV128,
		Div256 = RCC_CFGR_HPRE_DIV256,
		Div512 = RCC_CFGR_HPRE_DIV512
	};

	enum class
	Apb1Prescaler : uint32_t
	{
		Div1   = RCC_CFGR_PPRE1_DIV1,
		Div2   = RCC_CFGR_PPRE1_DIV2,
		Div4   = RCC_CFGR_PPRE1_DIV4,
		Div8   = RCC_CFGR_PPRE1_DIV8,
		Div16  = RCC_CFGR_PPRE1_DIV16
	};

	enum class
	Apb2Prescaler : uint32_t
	{
		Div1   = RCC_CFGR_PPRE2_DIV1,
		Div2   = RCC_CFGR_PPRE2_DIV2,
		Div4   = RCC_CFGR_PPRE2_DIV4,
		Div8   = RCC_CFGR_PPRE2_DIV8,
		Div16  = RCC_CFGR_PPRE2_DIV16
	};
	enum class Clock48Source
	{
		PllQ = 0,
		PllSaiP = RCC_DCKCFGR2_CK48MSEL
	};
	enum class
	ClockOutput1Source : uint32_t
	{
		InternalClock = 0,
		ExternalClock = RCC_CFGR_MCO1_1,
		ExternalCrystal = RCC_CFGR_MCO1_1,
		Pll = RCC_CFGR_MCO1_1 | RCC_CFGR_MCO1_0,
	};

	enum class
	ClockOutput2Source : uint32_t
	{
		SystemClock = 0,
		ExternalClock = RCC_CFGR_MCO2_1,
		ExternalCrystal = RCC_CFGR_MCO2_1,
		Pll = RCC_CFGR_MCO2_1 | RCC_CFGR_MCO2_0,
	};
public:
	// sources
	static bool
	enableInternalClock(uint32_t waitCycles = 2048);

	static bool
	enableExternalClock(uint32_t waitCycles = 2048);

	static bool
	enableExternalCrystal(uint32_t waitCycles = 2048);

	static bool
	enableLowSpeedInternalClock(uint32_t waitCycles = 2048);

	static bool
	enableLowSpeedExternalClock(uint32_t waitCycles = 2048);

	static bool
	enableLowSpeedExternalCrystal(uint32_t waitCycles = 2048);

	struct PllFactors
	{
		uint8_t pllM;
		uint16_t pllN;
		uint8_t pllP;
		uint8_t pllQ = 0xff;
	};
	struct PllSaiFactors
	{
		const uint8_t pllSaiM;
		const uint16_t pllSaiN;
		const uint8_t pllSaiP;
	};
	/**
	 * Enable PLL.
	 *
	 * \param	source
	 * 		Source select for PLL. If you are using HSE you must
	 * 		enable it first (see enableHse()).
	 *
	 * \param	factors
	 * 		Struct with all pll factors. \see PllFactors.
	 *
	 * \param	waitCycles
	 * 		Number of cycles to wait for the pll to stabilise. Default: 2048.
	 */
	static bool
	enablePll(PllSource source, const PllFactors& pllFactors, uint32_t waitCycles = 2048);

	/**
	 * Disable PLL.
	 *
	 * \param	waitCycles
	 * 		Number of cycles to wait for the pll to stabilise. Default: 2048.
	 */
	static bool
	disablePll(uint32_t waitCycles = 2048);
	/**
	 * Enable PLLSAI.
	 *
	 * \warning The PLL source must be selected first by configuring the main PLL.
	 *
	 * \param	factors
	 * 		Struct with all pllsai factors. \see PllSaiFactors.
	 *
	 * \param	waitCycles
	 * 		Number of cycles to wait for the pll to stabilise. Default: 2048.
	 */
	static bool
	enablePllSai(const PllSaiFactors& pllFactors, uint32_t waitCycles = 2048);

	/**
	 * Disable PLLSAI.
	 *
	 * \param	waitCycles
	 * 		Number of cycles to wait for the pll to stabilise. Default: 2048.
	 */
	static bool
	disablePllSai(uint32_t waitCycles = 2048);
	// sinks
	static bool
	enableSystemClock(SystemClockSource src, uint32_t waitCycles = 2048);

	static inline bool
	enableRealTimeClock(RealTimeClockSource src)
	{
		RCC->BDCR = (RCC->BDCR & ~RCC_BDCR_RTCSEL) | RCC_BDCR_RTCEN | uint32_t(src);
		return true;
	}

	static inline bool
	enableWatchdogClock(WatchdogClockSource /*src*/)
	{ return true; }

	static inline void
	setClock48Source(Clock48Source source)
	{
		RCC->DCKCFGR2 = (RCC->DCKCFGR2 & ~RCC_DCKCFGR2_CK48MSEL) | uint32_t(source);
	}
	static inline bool
	enableClockOutput1(ClockOutput1Source src, uint8_t div)
	{
		uint32_t tmp = RCC->CFGR & ~(RCC_CFGR_MCO1 | RCC_CFGR_MCO1PRE);
		if (div > 1) tmp |= (div + 2) << 24;
		RCC->CFGR = tmp | uint32_t(src);
		return true;
	}

	static inline bool
	enableClockOutput2(ClockOutput2Source src, uint8_t div)
	{
		uint32_t tmp = RCC->CFGR & ~(RCC_CFGR_MCO2 | RCC_CFGR_MCO2PRE);
		if (div > 1) tmp |= (div + 2) << 27;
		RCC->CFGR = tmp | uint32_t(src);
		return true;
	}
public:
	static inline bool
	setAhbPrescaler(AhbPrescaler prescaler)
	{
		RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_HPRE) | uint32_t(prescaler);
		return true;
	}

	static inline bool
	setApb1Prescaler(Apb1Prescaler prescaler)
	{
		RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PPRE1) | uint32_t(prescaler);
		return true;
	}

	static inline bool
	setApb2Prescaler(Apb2Prescaler prescaler)
	{
		RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PPRE2) | uint32_t(prescaler);
		return true;
	}
	static bool
	enableOverdriveMode(uint32_t waitCycles = 2048);
public:
	/** Set flash latency for CPU frequency and voltage.
	 * Does nothing if CPU frequency is too high for the available
	 * voltage.
	 *
	 * @returns maximum CPU frequency for voltage.
	 * @retval	<=CPU_Frequency flash latency has been set correctly.
	 * @retval	>CPU_Frequency requested frequency too high for voltage.
	 */
	template< uint32_t Core_Hz, uint16_t Core_mV = 3300>
	static uint32_t
	setFlashLatency();

	template< uint32_t Core_Hz >
	static void
	updateCoreFrequency();

	template< class... Signals >
	static void
	connect()
	{
		using Connector = GpioConnector<Peripheral::Rcc, Signals...>;
		Connector::connect();
	}

public:
	template< Peripheral peripheral >
	static void
	enable();

	template< Peripheral peripheral >
	static bool
	isEnabled();

	template< Peripheral peripheral >
	static void
	disable();

private:
	struct flash_latency
	{
		uint32_t latency;
		uint32_t max_frequency;
	};
	static constexpr flash_latency
	computeFlashLatency(uint32_t Core_Hz, uint16_t Core_mV);
};

}   // namespace modm::platform


#include "rcc_impl.hpp"

#endif	//  MODM_STM32_RCC_HPP