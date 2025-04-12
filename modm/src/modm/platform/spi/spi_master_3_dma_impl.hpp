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
#	error 	"Don't include this file directly, use 'spi_master_3_dma.hpp' instead!"
#endif

template <class DmaChannelRx, class DmaChannelTx>
template <class SystemClock, modm::baudrate_t baudrate, modm::percent_t tolerance>
void
modm::platform::SpiMaster3_Dma<DmaChannelRx, DmaChannelTx>::initialize()
{
	// Configure the DMA channels, then calls SpiMaster3::initialzie().
	Dma::RxChannel::configure(DmaBase::DataTransferDirection::PeripheralToMemory,
			DmaBase::MemoryDataSize::Byte, DmaBase::PeripheralDataSize::Byte,
			DmaBase::MemoryIncrementMode::Increment, DmaBase::PeripheralIncrementMode::Fixed,
			DmaBase::Priority::High);
	Dma::RxChannel::setPeripheralAddress(SPI3_BASE + 0x0c);
	Dma::RxChannel::setTransferErrorIrqHandler(handleDmaTransferError);
	Dma::RxChannel::setTransferCompleteIrqHandler(handleDmaReceiveComplete);
	Dma::RxChannel::enableInterruptVector();
	Dma::RxChannel::enableInterrupt(DmaBase::InterruptEnable::TransferError |
			DmaBase::InterruptEnable::TransferComplete);
	Dma::RxChannel::template setPeripheralRequest<Dma::RxRequest>();

	Dma::TxChannel::configure(DmaBase::DataTransferDirection::MemoryToPeripheral,
			DmaBase::MemoryDataSize::Byte, DmaBase::PeripheralDataSize::Byte,
			DmaBase::MemoryIncrementMode::Increment, DmaBase::PeripheralIncrementMode::Fixed,
			DmaBase::Priority::High);
	Dma::TxChannel::setPeripheralAddress(SPI3_BASE + 0x0c);
	Dma::TxChannel::setTransferErrorIrqHandler(handleDmaTransferError);
	Dma::TxChannel::setTransferCompleteIrqHandler(handleDmaTransmitComplete);
	Dma::TxChannel::enableInterruptVector();
	Dma::TxChannel::enableInterrupt(DmaBase::InterruptEnable::TransferError |
			DmaBase::InterruptEnable::TransferComplete);
	Dma::TxChannel::template setPeripheralRequest<Dma::TxRequest>();

	SpiMaster3::initialize<SystemClock, baudrate, tolerance>();

}

template <class DmaChannelRx, class DmaChannelTx>
modm::ResumableResult<uint8_t>
modm::platform::SpiMaster3_Dma<DmaChannelRx, DmaChannelTx>::transfer(uint8_t data)
{
	// disable DMA for single byte transfer
	SpiHal3::disableInterrupt(SpiBase::Interrupt::TxDmaEnable |
			SpiBase::Interrupt::RxDmaEnable);

	// wait for previous transfer to finish
	while(!SpiHal3::isTransmitRegisterEmpty())
		modm::this_fiber::yield();

	// start transfer by copying data into register
	SpiHal3::write(data);

	// wait for current transfer to finish
	while(!SpiHal3::isReceiveRegisterNotEmpty())
		modm::this_fiber::yield();

	// read the received byte
	SpiHal3::read(data);

	return data;
}

template <class DmaChannelRx, class DmaChannelTx>
modm::ResumableResult<void>
modm::platform::SpiMaster3_Dma<DmaChannelRx, DmaChannelTx>::transfer(
		const uint8_t *tx, uint8_t *rx, std::size_t length)
{
	dmaError = false;
	SpiHal3::enableInterrupt(
			SpiBase::Interrupt::TxDmaEnable | SpiBase::Interrupt::RxDmaEnable);

	if (tx) {
		Dma::TxChannel::setMemoryAddress(uint32_t(tx));
		Dma::TxChannel::setMemoryIncrementMode(true);
	} else {
		Dma::TxChannel::setMemoryAddress(uint32_t(&dmaDummy));
		Dma::TxChannel::setMemoryIncrementMode(false);
	}
	if (rx) {
		Dma::RxChannel::setMemoryAddress(uint32_t(rx));
		Dma::RxChannel::setMemoryIncrementMode(true);
	} else {
		Dma::RxChannel::setMemoryAddress(uint32_t(&dmaDummy));
		Dma::RxChannel::setMemoryIncrementMode(false);
	}

	Dma::RxChannel::setDataLength(length);
	dmaReceiveComplete = false;
	Dma::RxChannel::start();

	Dma::TxChannel::setDataLength(length);
	dmaTransmitComplete = false;
	Dma::TxChannel::start();

	while (true)
	{
		if (dmaError) break;
		else if (not dmaTransmitComplete and not dmaReceiveComplete)
			modm::this_fiber::yield();
		else if (SpiHal3::getInterruptFlags() & SpiBase::InterruptFlag::Busy)
			modm::this_fiber::yield();
		else break;
	}

	SpiHal3::disableInterrupt(
			SpiBase::Interrupt::TxDmaEnable | SpiBase::Interrupt::RxDmaEnable);

}

template <class DmaChannelRx, class DmaChannelTx>
void
modm::platform::SpiMaster3_Dma<DmaChannelRx, DmaChannelTx>::handleDmaTransferError()
{
	SpiHal3::disableInterrupt(SpiBase::Interrupt::TxDmaEnable |
			SpiBase::Interrupt::RxDmaEnable);
	Dma::RxChannel::stop();
	Dma::TxChannel::stop();
	dmaError = true;
}

template <class DmaChannelRx, class DmaChannelTx>
void
modm::platform::SpiMaster3_Dma<DmaChannelRx, DmaChannelTx>::handleDmaReceiveComplete()
{
	Dma::RxChannel::stop();
	dmaReceiveComplete = true;
}

template <class DmaChannelRx, class DmaChannelTx>
void
modm::platform::SpiMaster3_Dma<DmaChannelRx, DmaChannelTx>::handleDmaTransmitComplete()
{
	Dma::TxChannel::stop();
	dmaTransmitComplete = true;
}