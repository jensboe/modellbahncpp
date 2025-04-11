#include "board.hpp"

int main()
{
    Board::initialize();

    Board::Adapter_A::LedRed::set(true);
    modm::delay(10ms);
    Board::Adapter_A::LedRed::set(false);
    Board::Adapter_A::LedYellow::set(true);
    modm::delay(10ms);
    Board::Adapter_A::LedYellow::set(false);
    Board::Adapter_A::LedGreen::set(true);
    while (true)
    {
        Board::Nucleo::LedBlue::toggle();
        modm::delay(50ms);
        static uint32_t counter(0);
        MODM_LOG_INFO << "Loop counter: " << (counter++) << modm::endl;
    }
    return 0;
}