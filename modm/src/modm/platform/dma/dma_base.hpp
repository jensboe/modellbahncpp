/*
 * Copyright (c) 2014, Kevin Läufer
 * Copyright (c) 2014-2017, Niklas Hauser
 * Copyright (c) 2020, Mike Wolfram
 * Copyright (c) 2021, Raphael Lehmann
 * Copyright (c) 2021, Christopher Durand
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#ifndef MODM_STM32_DMA_BASE_HPP
#define MODM_STM32_DMA_BASE_HPP

#include <stdint.h>
#include <cstddef>

#include "../device.hpp"

#include <modm/architecture/interface/assert.hpp>
#include <modm/architecture/interface/interrupt.hpp>
#include <modm/architecture/interface/register.hpp>

namespace modm
{

namespace platform
{
/**
 * DMA
 *
 * @author	Kevin Laeufer
 * @author	Mike Wolfram
 * @ingroup	modm_platform_dma
 */
class DmaBase
{
public:
	// Enums
	enum class
	Channel
	{
		Stream0 = 0,
		Stream1,
		Stream2,
		Stream3,
		Stream4,
		Stream5,
		Stream6,
		Stream7,
		Channel0 = Stream0,
		Channel1 = Stream1,
		Channel2 = Stream2,
		Channel3 = Stream3,
		Channel4 = Stream4,
		Channel5 = Stream5,
		Channel6 = Stream6,
		Channel7 = Stream7,
	};

	enum class
	Request : uint32_t
	{
		Channel0 = (0 << DMA_SxCR_CHSEL_Pos),
		Channel1 = (1 << DMA_SxCR_CHSEL_Pos),
		Channel2 = (2 << DMA_SxCR_CHSEL_Pos),
		Channel3 = (3 << DMA_SxCR_CHSEL_Pos),
		Channel4 = (4 << DMA_SxCR_CHSEL_Pos),
		Channel5 = (5 << DMA_SxCR_CHSEL_Pos),
		Channel6 = (6 << DMA_SxCR_CHSEL_Pos),
		Channel7 = (7 << DMA_SxCR_CHSEL_Pos),
	};

	enum class
	Priority : uint32_t
	{
		Low 		= 0,
		Medium  	= DMA_SxCR_PL_0,
		High 		= DMA_SxCR_PL_1,
		VeryHigh 	= DMA_SxCR_PL_1 | DMA_SxCR_PL_0,
	};

	/// In direct mode (if the FIFO is not used)
	/// MSIZE is forced by hardware to the same value as PSIZE
	enum class
	MemoryDataSize : uint32_t
	{
		Byte 		= 0,
		Bit8 		= Byte,
		HalfWord 	= DMA_SxCR_MSIZE_0,
		Bit16 		= HalfWord,
		Word 		= DMA_SxCR_MSIZE_1,
		Bit32 		= Word,
	};

	enum class
	PeripheralDataSize : uint32_t
	{
		Byte 		= 0,
		Bit8 		= Byte,
		HalfWord 	= DMA_SxCR_PSIZE_0,
		Bit16 		= HalfWord,
		Word 		= DMA_SxCR_PSIZE_1,
		Bit32 		= Word,
	};

	enum class
	MemoryIncrementMode : uint32_t
	{
		Fixed 		= 0,
		Increment 	= DMA_SxCR_MINC, ///< incremented according to MemoryDataSize
	};

	enum class
	PeripheralIncrementMode : uint32_t
	{
		Fixed 		= 0,
		Increment 	= DMA_SxCR_PINC, ///< incremented according to PeripheralDataSize
	};

	enum class
	CircularMode : uint32_t
	{
		Disabled 	= 0,
		Enabled 	= DMA_SxCR_CIRC, ///< circular mode
	};

	enum class
	DataTransferDirection : uint32_t
	{
		/// Source: DMA_SxPAR; Sink: DMA_SxM0AR
		PeripheralToMemory 	= 0,
		/// Source: DMA_SxM0AR; Sink: DMA_SxPAR
		MemoryToPeripheral 	= DMA_SxCR_DIR_0,
		/// Source: DMA_SxPAR; Sink: DMA_SxM0AR
		MemoryToMemory 		= DMA_SxCR_DIR_1,
	};

	/**
	 * Peripheral signals that can be used in DMA channels
	 */
	enum class
	Signal : uint8_t {
		NoSignal,
		A,
		B,
		Ch1,
		Ch2,
		Ch3,
		Ch4,
		Com,
		Cs,
		Dac1,
		Dac2,
		Dt,
		Rx,
		Trig,
		Tx,
		Up,
	};

	enum class InterruptEnable : uint32_t {
		DirectModeError		= DMA_SxCR_DMEIE,
		TransferError		= DMA_SxCR_TEIE,
		HalfTransfer		= DMA_SxCR_HTIE,
		TransferComplete	= DMA_SxCR_TCIE,
	};
	MODM_FLAGS32(InterruptEnable);

	enum class InterruptFlags : uint8_t {
		FifoError = 0b00'0001,
		DirectModeError = 0b00'0100,
		Error = 0b00'1000,
		HalfTransferComplete = 0b01'0000,
		TransferComplete = 0b10'0000,
		All = 0b11'1101,
		Global = All,
	};
	MODM_FLAGS32(InterruptFlags);
	using IrqHandler = void (*)(void);

protected:
	static constexpr uint32_t memoryMask =
			DMA_SxCR_MBURST_Msk |					// MemoryBurstTransfer
			DMA_SxCR_MSIZE_Msk |					// MemoryDataSize
			DMA_SxCR_MINC |							// MemoryIncrementMode
			DMA_SxCR_DIR_Msk;    					// DataTransferDirection
	static constexpr uint32_t peripheralMask =
			DMA_SxCR_PBURST_Msk |					// PeripheralBurstTransfer
			DMA_SxCR_PSIZE_Msk |					// PeripheralDataSize
			DMA_SxCR_PINC_Msk |						// PeripheralIncrementMode
			DMA_SxCR_DIR_Msk;						// DataTransferDirection
	static constexpr uint32_t configmask =
			DMA_SxCR_CHSEL_Msk |					// Channel
			DMA_SxCR_PL_Msk |						// Priority
			DMA_SxCR_CIRC_Msk |						// CircularMode
			DMA_SxCR_PFCTRL_Msk;					// FlowControl
	template <uint32_t ID>
	struct Nvic;
};

template <>
struct DmaBase::Nvic<1>
{
	static constexpr IRQn_Type DmaIrqs[] {
		DMA1_Stream0_IRQn,
		DMA1_Stream1_IRQn,
		DMA1_Stream2_IRQn,
		DMA1_Stream3_IRQn,
		DMA1_Stream4_IRQn,
		DMA1_Stream5_IRQn,
		DMA1_Stream6_IRQn,
		DMA1_Stream7_IRQn,
	};
};

template <>
struct DmaBase::Nvic<2>
{
	static constexpr IRQn_Type DmaIrqs[] {
		DMA2_Stream0_IRQn,
		DMA2_Stream1_IRQn,
		DMA2_Stream2_IRQn,
		DMA2_Stream3_IRQn,
		DMA2_Stream4_IRQn,
		DMA2_Stream5_IRQn,
		DMA2_Stream6_IRQn,
		DMA2_Stream7_IRQn,
	};
};

}	// namespace platform

}	// namespace modm

#endif	// MODM_STM32_DMA_BASE_HPP