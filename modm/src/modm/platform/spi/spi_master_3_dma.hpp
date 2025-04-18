/*
 * Copyright (c) 2020, Mike Wolfram
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#ifndef MODM_STM32_SPI_MASTER3_DMA_HPP
#define MODM_STM32_SPI_MASTER3_DMA_HPP

#include <modm/platform/dma/dma.hpp>
#include "spi_master_3.hpp"

namespace modm
{

namespace platform
{

/**
 * Serial peripheral interface (SPI3) with DMA support.
 *
 * This class uses the DMA controller for sending and receiving data, which
 * greatly reduces the CPU load. Beside passing the DMA channels as template
 * parameters the class can be used in the same way like the SpiMaster3.
 *
 * @tparam DmaChannelRX DMA channel for receiving
 * @tparam DmaChannelTX DMA channel for sending
 *
 * @author	Mike Wolfram
 * @ingroup	modm_platform_spi modm_platform_spi_3
 */
template <class DmaChannelRx, class DmaChannelTx>
class SpiMaster3_Dma : public SpiMaster3
{
	struct Dma {
		using RxChannel = typename DmaChannelRx::template RequestMapping<
				Peripheral::Spi3, DmaBase::Signal::Rx>::Channel;
		using TxChannel = typename DmaChannelTx::template RequestMapping<
				Peripheral::Spi3, DmaBase::Signal::Tx>::Channel;
		static constexpr DmaBase::Request RxRequest = DmaChannelRx::template RequestMapping<
				Peripheral::Spi3, DmaBase::Signal::Rx>::Request;
		static constexpr DmaBase::Request TxRequest = DmaChannelTx::template RequestMapping<
				Peripheral::Spi3, DmaBase::Signal::Tx>::Request;
	};

public:
	/**
	 * Initialize DMA and SPI3
	 */
	template <class SystemClock, baudrate_t baudrate, percent_t tolerance=pct(5)>
	static void
	initialize();

	static uint8_t
	transferBlocking(uint8_t data)
	{
		return RF_CALL_BLOCKING(transfer(data));
	}

	static void
	transferBlocking(const uint8_t *tx, uint8_t *rx, std::size_t length)
	{
		RF_CALL_BLOCKING(transfer(tx, rx, length));
	}

	static modm::ResumableResult<uint8_t>
	transfer(uint8_t data);

	static modm::ResumableResult<void>
	transfer(const uint8_t *tx, uint8_t *rx, std::size_t length);

private:
	static void
	handleDmaTransferError();
	static void
	handleDmaReceiveComplete();
	static void
	handleDmaTransmitComplete();

	static inline bool dmaError { false };
	static inline bool dmaTransmitComplete { false };
	static inline bool dmaReceiveComplete { false };

	// needed for transfers where no RX or TX buffers are given
	static inline uint8_t dmaDummy { 0 };
};

} // namespace platform

} // namespace modm

#include "spi_master_3_dma_impl.hpp"

#endif // MODM_STM32_SPI_MASTER3_DMA_HPP