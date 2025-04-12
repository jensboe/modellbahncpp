#include "board.hpp"

#include "track/straight.hpp"
#include "track/switch.hpp"
#include "expansion/controller.hpp"
int main()
{
    Board::initialize();
    controller controller;

    Board::Adapter_A::LedRed::set(true);
    modm::delay(100ms);
    Board::Adapter_A::LedRed::set(false);
    Board::Adapter_A::LedYellow::set(true);
    modm::delay(10ms);
    Board::Adapter_A::LedYellow::set(false);
    Board::Adapter_A::LedGreen::set(true);

    static const auto tracks = std::to_array<std::shared_ptr<track>>({
        std::make_shared<straight>(0, "Track  1", ioposition(0, 0), 200, 4, 1),
        std::make_shared<switch_track>(1, "Switch 1", ioposition(0, 1), 200, 2, 3, 0, ioposition(0, 5), ioposition(0, 6)),
        std::make_shared<straight>(2, "Track  2a", ioposition(0, 2), 200, 1, 4),
        std::make_shared<straight>(3, "Track  2b", ioposition(0, 3), 200, 1, 4),
        std::make_shared<switch_track>(4, "Switch 2", ioposition(0, 4), 200, 2, 3, 0, ioposition(0, 7), ioposition(1, 0)),
    });

    auto last_track = tracks[0];
    auto current_track = tracks[1];

    while (true)
    {
        auto next_id = current_track->next_track(last_track->id);
        if (next_id < 0 || next_id >= static_cast<int>(tracks.size()))
        {
            MODM_LOG_ERROR << "Next track not found " << current_track->name << " comming from  " << last_track->name << modm::endl;
            break;
        }
        auto next_track = tracks[next_id];

        MODM_LOG_INFO << "Current: " << current_track->name << "\tNext: " << next_track->name << "\tLast: " << last_track->name << modm::endl;

        last_track = current_track;
        current_track = next_track;
        last_track->powerstate = power::OFF;
        current_track->powerstate = power::ON;

        for (const auto &track : tracks)
        {
            controller.set_buffer(track->power_pos, track->powerstate == power::ON);
        }
        controller.update();
        Board::Nucleo::LedBlue::toggle();
        modm::delay(500ms);
    }
    MODM_LOG_ERROR << "Abandoning...\n"
                   << modm::flush;
    while (true)
    {
    }
    return 0;
}