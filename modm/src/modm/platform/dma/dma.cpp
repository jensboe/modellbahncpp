/*
 * Copyright (c) 2020, Mike Wolfram
 * Copyright (c) 2021, Christopher Durand
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include "dma.hpp"

using namespace modm::platform;

MODM_ISR(DMA1_Stream0)
{
	Dma1::Channel<DmaBase::Channel::Channel0>::interruptHandler();
}

MODM_ISR(DMA1_Stream1)
{
	Dma1::Channel<DmaBase::Channel::Channel1>::interruptHandler();
}

MODM_ISR(DMA1_Stream2)
{
	Dma1::Channel<DmaBase::Channel::Channel2>::interruptHandler();
}

MODM_ISR(DMA1_Stream3)
{
	Dma1::Channel<DmaBase::Channel::Channel3>::interruptHandler();
}

MODM_ISR(DMA1_Stream4)
{
	Dma1::Channel<DmaBase::Channel::Channel4>::interruptHandler();
}

MODM_ISR(DMA1_Stream5)
{
	Dma1::Channel<DmaBase::Channel::Channel5>::interruptHandler();
}

MODM_ISR(DMA1_Stream6)
{
	Dma1::Channel<DmaBase::Channel::Channel6>::interruptHandler();
}

MODM_ISR(DMA1_Stream7)
{
	Dma1::Channel<DmaBase::Channel::Channel7>::interruptHandler();
}

MODM_ISR(DMA2_Stream0)
{
	Dma2::Channel<DmaBase::Channel::Channel0>::interruptHandler();
}

MODM_ISR(DMA2_Stream1)
{
	Dma2::Channel<DmaBase::Channel::Channel1>::interruptHandler();
}

MODM_ISR(DMA2_Stream2)
{
	Dma2::Channel<DmaBase::Channel::Channel2>::interruptHandler();
}

MODM_ISR(DMA2_Stream3)
{
	Dma2::Channel<DmaBase::Channel::Channel3>::interruptHandler();
}

MODM_ISR(DMA2_Stream4)
{
	Dma2::Channel<DmaBase::Channel::Channel4>::interruptHandler();
}

MODM_ISR(DMA2_Stream5)
{
	Dma2::Channel<DmaBase::Channel::Channel5>::interruptHandler();
}

MODM_ISR(DMA2_Stream6)
{
	Dma2::Channel<DmaBase::Channel::Channel6>::interruptHandler();
}

MODM_ISR(DMA2_Stream7)
{
	Dma2::Channel<DmaBase::Channel::Channel7>::interruptHandler();
}

