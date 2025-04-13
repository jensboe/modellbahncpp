#include "board.hpp"

#include "expansion/controller.hpp"
#include <modm/processing.hpp>
#include <modm/driver/adc/adc_sampler.hpp>

modm::Fiber measurement(
    []
    {
        typedef GpioInputA3 AdcCurrent;
        typedef GpioInputC0 AdcVoltage;
        // AdcVoltage::In10
        // AdcCurrent::In3
        Adc1::Channel sensorMapping[3] = {
            Adc1::getPinChannel<AdcCurrent>(),
            Adc1::getPinChannel<AdcVoltage>(),
            Adc1::Channel::TemperatureSensor,
        };
        uint32_t sensorData[3];

        typedef modm::AdcSampler<AdcInterrupt1, 3, 100> sensors;

        Adc1::connect<AdcCurrent::In3, AdcVoltage::In10>();
        Adc1::initialize<Board::SystemClock, 11'250_kHz>();

        Adc1::enableInterruptVector(5);
        Adc1::enableInterrupt(Adc1::Interrupt::EndOfRegularConversion);

        sensors::initialize(sensorMapping, sensorData);

        while (true)
        {
            sensors::startReadout();
            modm::this_fiber::poll(sensors::isReadoutFinished);
            uint32_t *data = sensors::getData();

            MODM_LOG_INFO << "current=" << data[0];
            MODM_LOG_INFO << "\tvoltage=" << data[1];
            MODM_LOG_INFO << "\ttemperature=" << data[2] << modm::endl;
            modm::this_fiber::sleep_for(100ms);
        }
    });
int main()
{
    Board::initialize();

    Board::Adapter_A::LedRed::set(true);
    modm::delay(500ms);
    Board::Adapter_A::LedRed::set(false);
    Board::Adapter_A::LedYellow::set(true);
    modm::delay(200ms);
    Board::Adapter_A::LedYellow::set(false);
    Board::Adapter_A::LedGreen::set(true);

    modm::fiber::Scheduler::run();
    return 0;
}
