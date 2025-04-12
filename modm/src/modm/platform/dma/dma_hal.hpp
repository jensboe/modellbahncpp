/*
 * Copyright (c) 2020, Mike Wolfram
 * Copyright (c) 2021, Raphael Lehmann
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#ifndef MODM_STM32_DMA_HAL_HPP
#define MODM_STM32_DMA_HAL_HPP

#include <modm/architecture/interface/assert.hpp>

#include "dma_base.hpp"

namespace modm
{

namespace platform
{

using DMA_Channel_TypeDef = DMA_Stream_TypeDef;
/**
 * Hardware abstraction of DMA controller
 *
 * @tparam ID The number of the DMA controller
 *
 * @author	Mike Wolfram
 * @ingroup	modm_platform_dma
 */
template <uint32_t ID>
class DmaHal : public DmaBase
{
	static_assert(ID >= 1 and ID <= 2,
			"invalid DMA controller ID");

	/**
	 * Get the base address of the DMA controller registers
	 *
	 * @tparam id The number of the DMA controller
	 */
	template <uint32_t id>
	static constexpr uint32_t getBaseAddress() {
		if constexpr (id == 1)
			return DMA1_BASE;
		else
			return DMA2_BASE;
	}
	/**
	 * Get the base address of the DMA channel registers
	 *
	 * @tparam id The number of the DMA channel
	 */
	template <uint32_t id>
	static constexpr uint32_t getChannelBaseAddress() {
		if (id == 1)
			return DMA1_Stream0_BASE;
		else
			return DMA2_Stream0_BASE;
	}

public:
	/// DMA base register address
	static constexpr uint32_t DMA_BASE { getBaseAddress<ID>() };
	/// DMA channel base register address
	static constexpr uint32_t CHANNEL_BASE { getChannelBaseAddress<ID>() };

	/// Register offset from channel to channel
	static constexpr uint32_t CHANNEL_2_CHANNEL { 0x18 };
	static void
	clearInterruptFlags(InterruptFlags flags, DmaBase::Channel ChannelID)
	{
		DMA_TypeDef *DMA = reinterpret_cast<DMA_TypeDef *>(DMA_BASE);
		uint8_t ch = uint8_t(ChannelID);
		constexpr uint8_t offsetLut[4] = {0, 6, 16, 22};
		if (ch < 4) {
			DMA->LIFCR = uint32_t(flags) << offsetLut[ch];
		}
		else if (ch < 8) {
			DMA->HIFCR = uint32_t(flags) << offsetLut[ch-4];
		}
	}

	static uint64_t
	getInterruptFlags()
	{
		DMA_TypeDef *DMA = reinterpret_cast<DMA_TypeDef *>(DMA_BASE);
		return DMA->LISR | (static_cast<uint64_t>(DMA->HISR) << 32);
	}
};

/**
 * Hardware abstraction layer of a DMA channel
 *
 * @tparam ChannelID the ID of the channel
 * @tparam CHANNEL_BASE base address of the channel registers
 *
 * @author	Mike Wolfram
 * @ingroup	modm_platform_dma
 */
template <DmaBase::Channel ChannelID, uint32_t CHANNEL_BASE>
class DmaChannelHal : public DmaBase
{
public:
	/**
	 * Configure the DMA channel (HAL)
	 *
	 * Stops the DMA channel and writes the new values to its control register.
	 *
	 * @param[in] direction Direction of the DMA channel
	 * @param[in] memoryDataSize Size of data in memory (byte, halfword, word)
	 * @param[in] peripheralDataSize Size of data in peripheral (byte, halfword, word)
	 * @param[in] memoryIncrement Defines whether the memory address is incremented
	 * 			  after a transfer completed
	 * @param[in] peripheralIncrement Defines whether the peripheral address is
	 * 			  incremented after a transfer completed
	 * @param[in] priority Priority of the DMA channel
	 * @param[in] circularMode Transfer data in circular mode?
	 */
	static void
	configure(DataTransferDirection direction, MemoryDataSize memoryDataSize,
			PeripheralDataSize peripheralDataSize,
			MemoryIncrementMode memoryIncrement,
			PeripheralIncrementMode peripheralIncrement,
			Priority priority = Priority::Medium,
			CircularMode circularMode = CircularMode::Disabled)
	{
		stop();

		DMA_Channel_TypeDef *Base = (DMA_Channel_TypeDef *) CHANNEL_BASE;
		Base->CR = uint32_t(direction) | uint32_t(memoryDataSize) |
				uint32_t(peripheralDataSize) | uint32_t(memoryIncrement) |
				uint32_t(peripheralIncrement) | uint32_t(priority) |
				uint32_t(circularMode);
	}

	/**
	 * Start the transfer of the DMA channel
	 */
	static void
	start();
	/**
	 * Stop a DMA channel transfer
	 */
	static void
	stop();

	/**
	 * Get the direction of the data transfer
	 */
	static DataTransferDirection
	getDataTransferDirection();

	/**
	 * Set the memory address of the DMA channel
	 *
	 * @note In Mem2Mem mode use this method to set the memory source address.
	 *
	 * @param[in] address Source address
	 */
	static void
	setMemoryAddress(uintptr_t address)
	{
		DMA_Channel_TypeDef *Base = (DMA_Channel_TypeDef *) CHANNEL_BASE;
		Base->M0AR = address;
	}

	/**
	 * Set the peripheral address of the DMA channel
	 *
	 * @note In Mem2Mem mode use this method to set the memory destination address.
	 *
	 * @param[in] address Destination address
	 */
	static void
	setPeripheralAddress(uintptr_t address)
	{
		DMA_Channel_TypeDef *Base = (DMA_Channel_TypeDef *) CHANNEL_BASE;
		Base->PAR = address;
	}

	/**
	 * Enable/disable memory increment
	 *
	 * When enabled, the memory address is incremented by the size of the data
	 * (e.g. 1 for byte transfers, 4 for word transfers) after the transfer
	 * completed.
	 *
	 * @param[in] increment Enable/disable
	 */
	static void
	setMemoryIncrementMode(bool increment)
	{
		DMA_Channel_TypeDef *Base = (DMA_Channel_TypeDef *) CHANNEL_BASE;
		if (increment)
			Base->CR |= uint32_t(MemoryIncrementMode::Increment);
		else
			Base->CR &= ~uint32_t(MemoryIncrementMode::Increment);
	}

	/**
	 * Enable/disable peripheral increment
	 *
	 * When enabled, the peripheral address is incremented by the size of the data
	 * (e.g. 1 for byte transfers, 4 for word transfers) after the transfer
	 * completed.
	 *
	 * @param[in] increment Enable/disable
	 */
	static void
	setPeripheralIncrementMode(bool increment)
	{
		DMA_Channel_TypeDef *Base = (DMA_Channel_TypeDef *) CHANNEL_BASE;
		if (increment)
			Base->CR |= uint32_t(PeripheralIncrementMode::Increment);
		else
			Base->CR &= ~uint32_t(PeripheralIncrementMode::Increment);
	}

	/**
	 * Set length of data to transfer
	 */
	static void
	setDataLength(std::size_t length)
	{
		DMA_Channel_TypeDef *Base = (DMA_Channel_TypeDef *) CHANNEL_BASE;
		Base->NDTR = length;
	}

	/**
	 * Enable IRQ of this DMA channel (e.g. transfer complete or error)
	 */
	static void
	enableInterrupt(InterruptEnable_t irq)
	{
		DMA_Channel_TypeDef *Base = (DMA_Channel_TypeDef *) CHANNEL_BASE;
		Base->CR |= irq.value;
	}
	/**
	 * Disable IRQ of this DMA channel (e.g. transfer complete or error)
	 */
	static void
	disableInterrupt(InterruptEnable_t irq)
	{
		DMA_Channel_TypeDef *Base = (DMA_Channel_TypeDef *) CHANNEL_BASE;
		Base->CR &= ~(irq.value);
	}
};

} // namespace platform
} // namespace modm

#include "dma_hal_impl.hpp"

#endif // MODM_STM32_DMA_HAL_HPP