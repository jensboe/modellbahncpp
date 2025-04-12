/*
 * Copyright (c) 2014, Kevin Läufer
 * Copyright (c) 2014-2017, Niklas Hauser
 * Copyright (c) 2020, Mike Wolfram
 * Copyright (c) 2021, Raphael Lehmann
 * Copyright (c) 2021-2023, Christopher Durand
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#ifndef MODM_STM32_DMA_HPP
#define MODM_STM32_DMA_HPP

#include <cstdint>
#include <algorithm>
#include <array>
#include "../device.hpp"
#include "dma_hal.hpp"

namespace modm
{

namespace platform
{

/**
 * DMA controller
 *
 * Does not support - among other things - double buffering or FIFO usage
 *
 * @author	Mike Wolfram
 * @ingroup	modm_platform_dma
 */
template <uint32_t ID>
class DmaController : public DmaBase
{
	static_assert(ID >= 1 and ID <= 2, "invalid DMA controller ID");

public:
	/**
	 * Enable the DMA controller in the RCC
	 */
	static void
	enable()
	{
		if constexpr (ID == 1)
			Rcc::enable<Peripheral::Dma1>();
		else {
			Rcc::enable<Peripheral::Dma2>();
		}
	}
	/**
	 * Disable the DMA controller in the RCC
	 */
	static void
	disable()
	{
		if constexpr (ID == 1)
			Rcc::disable<Peripheral::Dma1>();
		else
			Rcc::disable<Peripheral::Dma2>();
	}

	/**
	 * Class representing a DMA channel/stream
	 */
	template <DmaBase::Channel ChannelID>
	class Channel : public DmaBase
	{
		static_assert(
				(ID == 1 and
						ChannelID >= DmaBase::Channel::Channel0 and
						ChannelID <= DmaBase::Channel::Channel7)
				or
				(ID == 2 and
						ChannelID >= DmaBase::Channel::Channel0 and
						ChannelID <= DmaBase::Channel::Channel7)
				, "invalid Channel ID for that DMA controller"
				);

		using ControlHal = DmaHal<ID>;

		static constexpr uint32_t CHANNEL_BASE { ControlHal::CHANNEL_BASE +
			uint32_t(ChannelID) * ControlHal::CHANNEL_2_CHANNEL };

		using ChannelHal = DmaChannelHal<ChannelID, CHANNEL_BASE>;

		static constexpr uint8_t FlagOffsetLut[8] = {0, 6, 16, 22, 0+32, 6+32, 16+32, 22+32};
	public:
		/**
		 * Configure the DMA channel
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
			ChannelHal::configure(direction, memoryDataSize, peripheralDataSize,
					memoryIncrement, peripheralIncrement, priority, circularMode);
		}

		/**
		 * Start the transfer of the DMA channel and clear all interrupt flags.
		 */
		static void
		start()
		{
			ControlHal::clearInterruptFlags(InterruptFlags::All, ChannelID);
			ChannelHal::start();
		}

		/**
		 * Stop a DMA channel transfer
		 */
		static void
		stop()
		{
			ChannelHal::stop();
		}

		/**
		 * Get the direction of the data transfer
		 */
		static DataTransferDirection
		getDataTransferDirection()
		{
			return ChannelHal::getDataTransferDirection();
		}

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
			ChannelHal::setMemoryAddress(address);
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
			ChannelHal::setPeripheralAddress(address);
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
			ChannelHal::setMemoryIncrementMode(increment);
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
			ChannelHal::setPeripheralIncrementMode(increment);
		}

		/**
		 * Set the length of data to be transfered
		 */
		static void
		setDataLength(std::size_t length)
		{
			ChannelHal::setDataLength(length);
		}

		/**
		 * Set the IRQ handler for transfer errors
		 *
		 * The handler will be called from the channels IRQ handler function
		 * when the IRQ status indicates an error occured.
		 */
		static void
		setTransferErrorIrqHandler(IrqHandler irqHandler)
		{
			transferError = irqHandler;
		}
		/**
		 * Set the IRQ handler for half transfer complete
		 *
		 * Called by the channels IRQ handler when the transfer is half complete.
		 */
		static void
		setHalfTransferCompleteIrqHandler(IrqHandler irqHandler)
		{
			halfTransferComplete = irqHandler;
		}
		/**
		 * Set the IRQ handler for transfer complete
		 *
		 * Called by the channels IRQ handler when the transfer is complete.
		 */
		static void
		setTransferCompleteIrqHandler(IrqHandler irqHandler)
		{
			transferComplete = irqHandler;
		}

		/**
		 * Set the peripheral that operates the channel
		 */
		template <DmaBase::Request dmaRequest>
		static void
		setPeripheralRequest()
		{
			DMA_Channel_TypeDef *Channel = reinterpret_cast<DMA_Channel_TypeDef*>(CHANNEL_BASE);
			Channel->CR = (Channel->CR & ~DMA_SxCR_CHSEL_Msk) | uint32_t(dmaRequest);
		}

		/**
		 * IRQ handler of the DMA channel
		 *
		 * Reads the IRQ status and checks for error or transfer complete. In case
		 * of error the DMA channel will be disabled.
		 */
		modm_always_inline static void
		interruptHandler()
		{
			static const uint64_t HT_Flag {
				uint64_t(InterruptFlags::HalfTransferComplete) << FlagOffsetLut[uint32_t(ChannelID)]
			};
			static const uint64_t TC_Flag {
				uint64_t(InterruptFlags::TransferComplete) << FlagOffsetLut[uint32_t(ChannelID)]
			};
			static const uint64_t TE_Flag {
				uint64_t(InterruptFlags::Error) << FlagOffsetLut[uint32_t(ChannelID)]
			};
			auto isr { ControlHal::getInterruptFlags() };
			if (isr & TE_Flag) {
				disable();
				if (transferError)
					transferError();
			}
			if (halfTransferComplete and (isr & HT_Flag)) {
				halfTransferComplete();
			}
			if (transferComplete and (isr & TC_Flag)) {
				transferComplete();
			}

			ControlHal::clearInterruptFlags(InterruptFlags::Global, ChannelID);
		}

		/**
		 * Read channel status flags when channel interrupts are disabled.
		 * This function is useful to query the transfer state when the use of
		 * the channel interrupt is not required for the application.
		 *
		 * @warning Flags are automatically cleared in the ISR if the channel
		 * 			interrupt is enabled or when start() is called.
		 */
		static InterruptFlags_t
		getInterruptFlags()
		{
			const auto globalFlags = ControlHal::getInterruptFlags();
			const auto mask = static_cast<uint8_t>(InterruptFlags::All);
			const auto shift = FlagOffsetLut[uint32_t(ChannelID)];
			const auto channelFlags = static_cast<uint8_t>((globalFlags >> shift) & mask);
			return InterruptFlags_t{channelFlags};
		}

		/**
		 * Clear channel interrupt flags.
		 * Use only when the channel interrupt is disabled.
		 *
		 * @warning Flags are automatically cleared in the ISR if the channel
		 * 			interrupt is enabled or when start() is called.
		 */
		static void
		clearInterruptFlags(InterruptFlags_t flags = InterruptFlags::All)
		{
			ControlHal::clearInterruptFlags(InterruptFlags(flags.value), ChannelID);
		}

		/**
		 * Enable the IRQ vector of the channel
		 *
		 * @param[in] priority Priority of the IRQ
		 */
		static void
		enableInterruptVector(uint32_t priority = 1)
		{
			NVIC_SetPriority(DmaBase::Nvic<ID>::DmaIrqs[uint32_t(ChannelID)], priority);
			NVIC_EnableIRQ(DmaBase::Nvic<ID>::DmaIrqs[uint32_t(ChannelID)]);
		}
		/**
		 * Disable the IRQ vector of the channel
		 */
		static void
		disableInterruptVector()
		{
			NVIC_DisableIRQ(DmaBase::Nvic<ID>::DmaIrqs[uint32_t(ChannelID)]);
		}

		/**
		 * Enable the specified interrupt of the channel
		 */
		static void
		enableInterrupt(InterruptEnable_t irq)
		{
			ChannelHal::enableInterrupt(irq);
		}
		/**
		 * Disable the specified interrupt of the channel
		 */
		static void
		disableInterrupt(InterruptEnable_t irq)
		{
			ChannelHal::disableInterrupt(irq);
		}

		/**
		 * Helper to verify that the selected channel supports the selected
		 * hardware and provides the Request to be set in setPeripheralRequest().
		 */
		template <Peripheral peripheral, Signal signal = Signal::NoSignal>
		struct RequestMapping {
		};
	private:
		static inline DmaBase::IrqHandler transferError { nullptr };
		static inline DmaBase::IrqHandler halfTransferComplete { nullptr };
		static inline DmaBase::IrqHandler transferComplete { nullptr };

	};
};

/// @ingroup	modm_platform_dma
/// @{

/*
 * Derive DMA controller classes for convenience. Every derived class defines
 * the channels available on that controller.
 */
class Dma1: public DmaController<1>
{
public:
	using Channel0 = DmaController<1>::Channel<DmaBase::Channel::Channel0>;
	using Channel1 = DmaController<1>::Channel<DmaBase::Channel::Channel1>;
	using Channel2 = DmaController<1>::Channel<DmaBase::Channel::Channel2>;
	using Channel3 = DmaController<1>::Channel<DmaBase::Channel::Channel3>;
	using Channel4 = DmaController<1>::Channel<DmaBase::Channel::Channel4>;
	using Channel5 = DmaController<1>::Channel<DmaBase::Channel::Channel5>;
	using Channel6 = DmaController<1>::Channel<DmaBase::Channel::Channel6>;
	using Channel7 = DmaController<1>::Channel<DmaBase::Channel::Channel7>;
};
class Dma2: public DmaController<2>
{
public:
	using Channel0 = DmaController<2>::Channel<DmaBase::Channel::Channel0>;
	using Channel1 = DmaController<2>::Channel<DmaBase::Channel::Channel1>;
	using Channel2 = DmaController<2>::Channel<DmaBase::Channel::Channel2>;
	using Channel3 = DmaController<2>::Channel<DmaBase::Channel::Channel3>;
	using Channel4 = DmaController<2>::Channel<DmaBase::Channel::Channel4>;
	using Channel5 = DmaController<2>::Channel<DmaBase::Channel::Channel5>;
	using Channel6 = DmaController<2>::Channel<DmaBase::Channel::Channel6>;
	using Channel7 = DmaController<2>::Channel<DmaBase::Channel::Channel7>;
};
/// @}

/// @cond
/*
 * Specialization of the RequestMapping. For all hardware supported by DMA the
 * RequestMapping structure defines the channel and the Request. It can be used
 * by hardware classes to verify that the provided channel is valid and to
 * get the value to set in setPeripheralRequest().
 *
 * Example:
 * template <class DmaRx, class DmaTx>
 * class SpiMaster1_Dma : public SpiMaster1
 * {
 *     using RxChannel = typename DmaRx::template RequestMapping<Peripheral::Spi1, DmaBase::Signal::Rx>::Channel;
 * 	   using TxChannel = typename DmaTx::template RequestMapping<Peripheral::Spi1, DmaBase::Signal::Tx>::Channel;
 * 	   static constexpr DmaBase::Request RxRequest = DmaRx::template RequestMapping<Peripheral::Spi1, DmaBase::Signal::Rx>::Request;
 * 	   static constexpr DmaBase::Request TxRequest = DmaTx::template RequestMapping<Peripheral::Spi1, DmaBase::Signal::Tx>::Request;
 *
 *     ...
 * };
 */
template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel0>::RequestMapping<Peripheral::Spi3, DmaBase::Signal::Rx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel0>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel0>::RequestMapping<Peripheral::I2c1, DmaBase::Signal::Rx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel0>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel1;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel0>::RequestMapping<Peripheral::Tim4, DmaBase::Signal::Ch1>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel0>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel2;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel0>::RequestMapping<Peripheral::Uart5, DmaBase::Signal::Rx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel0>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel4;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel0>::RequestMapping<Peripheral::Tim5, DmaBase::Signal::Ch3>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel0>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel0>::RequestMapping<Peripheral::Tim5, DmaBase::Signal::Up>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel0>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel1>::RequestMapping<Peripheral::Spdifrx, DmaBase::Signal::Dt>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel1>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel1>::RequestMapping<Peripheral::I2c3, DmaBase::Signal::Rx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel1>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel1;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel1>::RequestMapping<Peripheral::Tim2, DmaBase::Signal::Ch3>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel1>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel3;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel1>::RequestMapping<Peripheral::Tim2, DmaBase::Signal::Up>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel1>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel3;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel1>::RequestMapping<Peripheral::Usart3, DmaBase::Signal::Rx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel1>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel4;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel1>::RequestMapping<Peripheral::Tim5, DmaBase::Signal::Ch4>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel1>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel1>::RequestMapping<Peripheral::Tim5, DmaBase::Signal::Trig>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel1>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel1>::RequestMapping<Peripheral::Tim6, DmaBase::Signal::Up>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel1>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel7;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel2>::RequestMapping<Peripheral::Spi3, DmaBase::Signal::Rx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel2>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel2>::RequestMapping<Peripheral::Tim7, DmaBase::Signal::Up>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel2>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel1;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel2>::RequestMapping<Peripheral::Fmpi2c1, DmaBase::Signal::Rx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel2>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel2;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel2>::RequestMapping<Peripheral::I2c3, DmaBase::Signal::Rx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel2>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel3;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel2>::RequestMapping<Peripheral::Uart4, DmaBase::Signal::Rx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel2>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel4;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel2>::RequestMapping<Peripheral::Tim3, DmaBase::Signal::Ch4>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel2>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel5;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel2>::RequestMapping<Peripheral::Tim3, DmaBase::Signal::Up>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel2>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel5;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel2>::RequestMapping<Peripheral::Tim5, DmaBase::Signal::Ch1>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel2>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel2>::RequestMapping<Peripheral::I2c2, DmaBase::Signal::Rx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel2>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel7;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel3>::RequestMapping<Peripheral::Spi2, DmaBase::Signal::Rx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel3>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel3>::RequestMapping<Peripheral::Tim4, DmaBase::Signal::Ch2>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel3>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel2;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel3>::RequestMapping<Peripheral::Usart3, DmaBase::Signal::Tx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel3>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel4;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel3>::RequestMapping<Peripheral::Tim5, DmaBase::Signal::Ch4>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel3>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel3>::RequestMapping<Peripheral::Tim5, DmaBase::Signal::Trig>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel3>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel3>::RequestMapping<Peripheral::I2c2, DmaBase::Signal::Rx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel3>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel7;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel4>::RequestMapping<Peripheral::Spi2, DmaBase::Signal::Tx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel4>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel4>::RequestMapping<Peripheral::Tim7, DmaBase::Signal::Up>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel4>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel1;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel4>::RequestMapping<Peripheral::I2c3, DmaBase::Signal::Tx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel4>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel3;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel4>::RequestMapping<Peripheral::Uart4, DmaBase::Signal::Tx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel4>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel4;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel4>::RequestMapping<Peripheral::Tim3, DmaBase::Signal::Ch1>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel4>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel5;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel4>::RequestMapping<Peripheral::Tim3, DmaBase::Signal::Trig>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel4>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel5;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel4>::RequestMapping<Peripheral::Tim5, DmaBase::Signal::Ch2>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel4>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel4>::RequestMapping<Peripheral::Usart3, DmaBase::Signal::Tx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel4>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel7;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel5>::RequestMapping<Peripheral::Spi3, DmaBase::Signal::Tx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel5>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel5>::RequestMapping<Peripheral::I2c1, DmaBase::Signal::Rx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel5>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel1;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel5>::RequestMapping<Peripheral::Fmpi2c1, DmaBase::Signal::Tx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel5>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel2;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel5>::RequestMapping<Peripheral::Tim2, DmaBase::Signal::Ch1>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel5>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel3;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel5>::RequestMapping<Peripheral::Usart2, DmaBase::Signal::Rx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel5>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel4;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel5>::RequestMapping<Peripheral::Tim3, DmaBase::Signal::Ch2>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel5>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel5;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel5>::RequestMapping<Peripheral::Dac, DmaBase::Signal::Dac1>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel5>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel7;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel6>::RequestMapping<Peripheral::Spdifrx, DmaBase::Signal::Cs>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel6>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel6>::RequestMapping<Peripheral::I2c1, DmaBase::Signal::Tx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel6>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel1;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel6>::RequestMapping<Peripheral::Tim4, DmaBase::Signal::Up>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel6>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel2;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel6>::RequestMapping<Peripheral::Tim2, DmaBase::Signal::Ch2>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel6>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel3;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel6>::RequestMapping<Peripheral::Tim2, DmaBase::Signal::Ch4>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel6>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel3;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel6>::RequestMapping<Peripheral::Usart2, DmaBase::Signal::Tx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel6>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel4;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel6>::RequestMapping<Peripheral::Tim5, DmaBase::Signal::Up>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel6>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel6>::RequestMapping<Peripheral::Dac, DmaBase::Signal::Dac2>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel6>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel7;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel7>::RequestMapping<Peripheral::Spi3, DmaBase::Signal::Tx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel7>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel7>::RequestMapping<Peripheral::I2c1, DmaBase::Signal::Tx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel7>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel1;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel7>::RequestMapping<Peripheral::Tim4, DmaBase::Signal::Ch3>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel7>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel2;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel7>::RequestMapping<Peripheral::Tim2, DmaBase::Signal::Ch4>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel7>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel3;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel7>::RequestMapping<Peripheral::Tim2, DmaBase::Signal::Up>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel7>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel3;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel7>::RequestMapping<Peripheral::Uart5, DmaBase::Signal::Tx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel7>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel4;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel7>::RequestMapping<Peripheral::Tim3, DmaBase::Signal::Ch3>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel7>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel5;
};

template <>
template <>
template <>
struct DmaController<1>::Channel<DmaBase::Channel::Channel7>::RequestMapping<Peripheral::I2c2, DmaBase::Signal::Tx>
{
	using Channel = DmaController<1>::Channel<DmaBase::Channel::Channel7>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel7;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel0>::RequestMapping<Peripheral::Adc1>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel0>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel0>::RequestMapping<Peripheral::Adc3>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel0>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel2;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel0>::RequestMapping<Peripheral::Spi1, DmaBase::Signal::Rx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel0>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel3;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel0>::RequestMapping<Peripheral::Spi4, DmaBase::Signal::Rx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel0>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel4;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel0>::RequestMapping<Peripheral::Tim1, DmaBase::Signal::Trig>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel0>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel1>::RequestMapping<Peripheral::Sai1, DmaBase::Signal::A>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel1>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel1>::RequestMapping<Peripheral::Dcmi>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel1>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel1;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel1>::RequestMapping<Peripheral::Adc3>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel1>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel2;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel1>::RequestMapping<Peripheral::Spi4, DmaBase::Signal::Tx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel1>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel4;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel1>::RequestMapping<Peripheral::Usart6, DmaBase::Signal::Rx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel1>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel5;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel1>::RequestMapping<Peripheral::Tim1, DmaBase::Signal::Ch1>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel1>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel1>::RequestMapping<Peripheral::Tim8, DmaBase::Signal::Up>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel1>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel7;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel2>::RequestMapping<Peripheral::Tim8, DmaBase::Signal::Ch2>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel2>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel2>::RequestMapping<Peripheral::Tim8, DmaBase::Signal::Ch3>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel2>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel2>::RequestMapping<Peripheral::Adc2>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel2>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel1;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel2>::RequestMapping<Peripheral::Spi1, DmaBase::Signal::Rx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel2>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel3;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel2>::RequestMapping<Peripheral::Usart1, DmaBase::Signal::Rx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel2>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel4;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel2>::RequestMapping<Peripheral::Usart6, DmaBase::Signal::Rx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel2>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel5;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel2>::RequestMapping<Peripheral::Tim1, DmaBase::Signal::Ch2>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel2>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel2>::RequestMapping<Peripheral::Tim8, DmaBase::Signal::Ch1>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel2>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel7;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel3>::RequestMapping<Peripheral::Sai1, DmaBase::Signal::A>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel3>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel3>::RequestMapping<Peripheral::Adc2>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel3>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel1;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel3>::RequestMapping<Peripheral::Spi1, DmaBase::Signal::Tx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel3>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel3;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel3>::RequestMapping<Peripheral::Sdio, DmaBase::Signal::Rx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel3>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel4;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel3>::RequestMapping<Peripheral::Sdio, DmaBase::Signal::Tx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel3>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel4;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel3>::RequestMapping<Peripheral::Spi4, DmaBase::Signal::Rx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel3>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel5;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel3>::RequestMapping<Peripheral::Tim1, DmaBase::Signal::Ch1>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel3>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel3>::RequestMapping<Peripheral::Tim8, DmaBase::Signal::Ch2>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel3>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel7;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel4>::RequestMapping<Peripheral::Adc1>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel4>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel4>::RequestMapping<Peripheral::Sai1, DmaBase::Signal::B>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel4>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel1;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel4>::RequestMapping<Peripheral::Sai2, DmaBase::Signal::A>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel4>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel3;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel4>::RequestMapping<Peripheral::Spi4, DmaBase::Signal::Tx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel4>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel5;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel4>::RequestMapping<Peripheral::Tim1, DmaBase::Signal::Ch4>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel4>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel4>::RequestMapping<Peripheral::Tim1, DmaBase::Signal::Com>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel4>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel4>::RequestMapping<Peripheral::Tim1, DmaBase::Signal::Trig>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel4>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel4>::RequestMapping<Peripheral::Tim8, DmaBase::Signal::Ch3>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel4>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel7;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel5>::RequestMapping<Peripheral::Sai1, DmaBase::Signal::B>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel5>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel5>::RequestMapping<Peripheral::Spi1, DmaBase::Signal::Tx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel5>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel3;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel5>::RequestMapping<Peripheral::Usart1, DmaBase::Signal::Rx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel5>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel4;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel5>::RequestMapping<Peripheral::Tim1, DmaBase::Signal::Up>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel5>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel6>::RequestMapping<Peripheral::Tim1, DmaBase::Signal::Ch1>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel6>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel6>::RequestMapping<Peripheral::Tim1, DmaBase::Signal::Ch2>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel6>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel6>::RequestMapping<Peripheral::Sai2, DmaBase::Signal::B>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel6>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel3;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel6>::RequestMapping<Peripheral::Sdio, DmaBase::Signal::Rx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel6>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel4;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel6>::RequestMapping<Peripheral::Sdio, DmaBase::Signal::Tx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel6>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel4;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel6>::RequestMapping<Peripheral::Usart6, DmaBase::Signal::Tx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel6>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel5;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel6>::RequestMapping<Peripheral::Tim1, DmaBase::Signal::Ch3>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel6>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel6;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel7>::RequestMapping<Peripheral::Sai2, DmaBase::Signal::B>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel7>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel0;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel7>::RequestMapping<Peripheral::Dcmi>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel7>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel1;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel7>::RequestMapping<Peripheral::Quadspi>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel7>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel3;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel7>::RequestMapping<Peripheral::Usart1, DmaBase::Signal::Tx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel7>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel4;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel7>::RequestMapping<Peripheral::Usart6, DmaBase::Signal::Tx>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel7>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel5;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel7>::RequestMapping<Peripheral::Tim8, DmaBase::Signal::Ch4>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel7>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel7;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel7>::RequestMapping<Peripheral::Tim8, DmaBase::Signal::Com>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel7>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel7;
};

template <>
template <>
template <>
struct DmaController<2>::Channel<DmaBase::Channel::Channel7>::RequestMapping<Peripheral::Tim8, DmaBase::Signal::Trig>
{
	using Channel = DmaController<2>::Channel<DmaBase::Channel::Channel7>;
	static constexpr DmaBase::Request Request = DmaBase::Request::Channel7;
};

/// @endcond
}	// namespace platform

}	// namespace modm

#endif	// MODM_STM32_DMA_HPP