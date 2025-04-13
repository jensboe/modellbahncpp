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
modm::Fiber driver_fiber(
    []
    {
        namespace L6226 = Board::Adapter_A::L6226;
        using Timer = L6226::Timer;
        using In1 = L6226::In1::Ch3;
        using In2 = L6226::In2::Ch1;

        auto const maxvalue = Timer::getOverflow();
        Timer::configureOutputChannel<In1>(Timer::OutputCompareMode::Pwm, 0);
        Timer::configureOutputChannel<In2>(Timer::OutputCompareMode::Pwm, 0);
        Timer::start();
        Timer::enableOutput();

        L6226::En::set();
        auto steper = 0;
        while (true)
        {
            if (steper > 0)
            {
                Timer::configureOutputChannel<In1>(Timer::OutputCompareMode::Pwm, maxvalue / 2);
                Timer::configureOutputChannel<In2>(Timer::OutputCompareMode::Pwm, 0);
            }
            else
            {
                Timer::configureOutputChannel<In1>(Timer::OutputCompareMode::Pwm, 0);
                Timer::configureOutputChannel<In2>(Timer::OutputCompareMode::Pwm, maxvalue / 2);
            }
            if (steper == 1)
            {
                steper = -1;
            }
            else
            {
                steper = 1;
            }

            modm::this_fiber::sleep_for(500us);
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
