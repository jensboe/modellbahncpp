#include "board.hpp"

#include "expansion/controller.hpp"
#include <modm/processing.hpp>
#include <modm/driver/adc/adc_sampler.hpp>

modm::Fiber measurement(
    []
    {
        using sensors = Board::Adapter_A::sensors;

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

    Board::Adapter_A::Indicator::LedRed::set(true);
    modm::delay(500ms);
    Board::Adapter_A::Indicator::LedRed::set(false);
    Board::Adapter_A::Indicator::LedYellow::set(true);
    modm::delay(200ms);
    Board::Adapter_A::Indicator::LedYellow::set(false);
    Board::Adapter_A::Indicator::LedGreen::set(true);

    modm::fiber::Scheduler::run();
    return 0;
}
