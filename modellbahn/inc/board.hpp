#pragma once

#include <modm/platform.hpp>
#include <modm/architecture/interface/clock.hpp>
#include <modm/debug/logger.hpp>

using namespace modm::platform;
using namespace modm::literals;
using namespace std::chrono_literals;

namespace Board
{
	using namespace modm::literals;

	/// STM32F446 running at 180 MHz from external 8 MHz STLink clock
	struct SystemClock
	{
		static constexpr uint32_t Frequency = 180_MHz;
		static constexpr uint32_t Ahb = Frequency;
		static constexpr uint32_t Apb1 = Frequency / 4;
		static constexpr uint32_t Apb2 = Frequency / 2;

		static constexpr uint32_t Adc = Apb2;

		static constexpr uint32_t Spi1 = Apb2;
		static constexpr uint32_t Spi2 = Apb1;
		static constexpr uint32_t Spi3 = Apb1;
		static constexpr uint32_t Spi4 = Apb2;
		static constexpr uint32_t Spi5 = Apb2;

		static constexpr uint32_t Usart1 = Apb2;
		static constexpr uint32_t Usart2 = Apb1;
		static constexpr uint32_t Usart3 = Apb1;
		static constexpr uint32_t Uart4 = Apb1;
		static constexpr uint32_t Uart5 = Apb1;
		static constexpr uint32_t Usart6 = Apb2;

		static constexpr uint32_t Can1 = Apb1;
		static constexpr uint32_t Can2 = Apb1;

		static constexpr uint32_t I2c1 = Apb1;
		static constexpr uint32_t I2c2 = Apb1;
		static constexpr uint32_t I2c3 = Apb1;

		static constexpr uint32_t Apb1Timer = Apb1 * 2;
		static constexpr uint32_t Apb2Timer = Apb2 * 1;
		static constexpr uint32_t Timer1 = Apb2Timer;
		static constexpr uint32_t Timer2 = Apb1Timer;
		static constexpr uint32_t Timer3 = Apb1Timer;
		static constexpr uint32_t Timer4 = Apb1Timer;
		static constexpr uint32_t Timer5 = Apb1Timer;
		static constexpr uint32_t Timer9 = Apb2Timer;
		static constexpr uint32_t Timer10 = Apb2Timer;
		static constexpr uint32_t Timer11 = Apb2Timer;

		static constexpr uint32_t Usb = 48_MHz;
		static constexpr uint32_t Iwdg = Rcc::LsiFrequency;
		static constexpr uint32_t Rtc = 32.768_kHz;

		static bool inline enable()
		{
			Rcc::enableLowSpeedExternalCrystal();
			Rcc::enableRealTimeClock(Rcc::RealTimeClockSource::LowSpeedExternalCrystal);

			Rcc::enableExternalClock(); // 8MHz
			const Rcc::PllFactors pllFactors{
				.pllM = 4,	 //  8MHz / M=  4 ->   2MHz
				.pllN = 180, //   2MHz * N=180 -> 360MHz
				.pllP = 2,	 // 360MHz / P=  2 -> 180MHz = F_cpu
			};
			Rcc::enablePll(Rcc::PllSource::ExternalClock, pllFactors);

			const Rcc::PllSaiFactors pllSaiFactors{
				.pllSaiM = 4,  //   8MHz / M=  4 ->   2MHz
				.pllSaiN = 96, //   2MHz * N= 96 -> 192MHz
				.pllSaiP = 4,  // 192MHz / P=  4 ->  48MHz = F_usb
			};
			Rcc::enablePllSai(pllSaiFactors);

			// "Overdrive" is required for guaranteed stable 180 MHz operation
			Rcc::enableOverdriveMode();
			Rcc::setFlashLatency<Frequency>();
			// switch system clock to PLL output
			Rcc::enableSystemClock(Rcc::SystemClockSource::Pll);
			Rcc::setAhbPrescaler(Rcc::AhbPrescaler::Div1);
			Rcc::setClock48Source(Rcc::Clock48Source::PllSaiP);
			// APB1 has max. 45MHz
			Rcc::setApb1Prescaler(Rcc::Apb1Prescaler::Div4);
			Rcc::setApb2Prescaler(Rcc::Apb2Prescaler::Div2);
			Rcc::updateCoreFrequency<Frequency>();

			return true;
		}
	};
	namespace Nucleo
	{
		using Button = GpioInputC13;

		using LedGreen = GpioOutputB0; // LED1 [Green]
		using LedBlue = GpioOutputB7;  // LED2 [Blue]
		using LedRed = GpioOutputB14;  // LED3 [Red]
		using Leds = SoftwareGpioPort<LedRed, LedBlue, LedGreen>;
	};
	namespace Adapter_A
	{
		using LedRed = GpioOutputC9;
		using LedYellow = GpioOutputG2;
		using LedGreen = GpioOutputG3;
		using SignalLeds = SoftwareGpioPort<LedRed, LedYellow, LedGreen>;
	};
	namespace ExpantionBoard
	{
		using SpiMaster = SpiMaster3;
		using Cs = GpioD2;
		using Sck = GpioC10;
		using Mosi = GpioC12;
		using Miso = GpioC11;
	};

	namespace usb
	{
		using Vbus = GpioA9;
		using Id = GpioA10;
		using Dm = GpioA11;
		using Dp = GpioA12;

		using Overcurrent = GpioInputG7; // OTG_FS_OverCurrent
		using Power = GpioOutputG6;		 // OTG_FS_PowerSwitchOn

		using Device = UsbFs;
	}

	namespace stlink
	{
		using Tx = GpioOutputD8;
		using Rx = GpioInputD9;
		using Uart = BufferedUart<UsartHal3, UartTxBuffer<2048>>;
	}

	using LoggerDevice = modm::IODeviceWrapper<modm::platform::Itm, modm::IOBuffer::DiscardIfFull>;

	inline void initialize()
	{
		SystemClock::enable();
		SysTickTimer::initialize<SystemClock>();

		stlink::Uart::connect<stlink::Tx::Tx, stlink::Rx::Rx>();
		stlink::Uart::initialize<SystemClock, 115200_Bd>();

		Nucleo::LedGreen::setOutput(modm::Gpio::Low);
		Nucleo::LedBlue::setOutput(modm::Gpio::Low);
		Nucleo::LedRed::setOutput(modm::Gpio::Low);

		Nucleo::Button::setInput();
		Adapter_A::LedRed::setOutput(modm::Gpio::Low);
		Adapter_A::LedYellow::setOutput(modm::Gpio::Low);
		Adapter_A::LedGreen::setOutput(modm::Gpio::Low);

		ExpantionBoard::SpiMaster::initialize<Board::SystemClock, 175_kHz>();
		ExpantionBoard::SpiMaster::setDataMode(ExpantionBoard::SpiMaster::DataMode::Mode0);
		ExpantionBoard::SpiMaster::connect<ExpantionBoard::Sck::Sck, ExpantionBoard::Mosi::Mosi, ExpantionBoard::Miso::Miso>();
		ExpantionBoard::Cs::setOutput(Gpio::OutputType::PushPull);
	}

	inline void initializeUsbFs(uint8_t priority = 3)
	{
		usb::Device::initialize<SystemClock>(priority);
		usb::Device::connect<usb::Dm::Dm, usb::Dp::Dp, usb::Id::Id>();

		usb::Overcurrent::setInput();
		usb::Vbus::setInput();
		// Force device mode
		USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_FDMOD;
		modm::delay_ms(25);
		// Enable VBUS sense (B device) via pin PA9
		USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_VBDEN;
	}
}
