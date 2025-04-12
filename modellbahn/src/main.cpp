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

    static const std::array<std::shared_ptr<track>, 12> tracks = {
        std::make_shared<straight>(0, "Track  1", ioposition(0, 0), 200, tracks.size() - 1, 1),
        std::make_shared<straight>(1, "Track  2", ioposition(0, 1), 200, 0, 2),
        std::make_shared<straight>(2, "Track  3", ioposition(0, 2), 200, 1, 3),
        std::make_shared<straight>(3, "Track  4", ioposition(0, 3), 200, 2, 4),
        std::make_shared<straight>(4, "Track  5", ioposition(0, 4), 200, 3, 5),
        std::make_shared<straight>(5, "Track  6", ioposition(0, 5), 200, 4, 6),
        std::make_shared<straight>(6, "Track  7", ioposition(0, 6), 200, 5, 7),
        std::make_shared<straight>(7, "Track  8", ioposition(0, 7), 200, 6, 8),
        std::make_shared<straight>(8, "Track  9", ioposition(1, 0), 200, 7, 9),
        std::make_shared<straight>(9, "Track  10", ioposition(1, 1), 200, 8, 10),
        std::make_shared<straight>(10, "Track  11", ioposition(1, 2), 200, 9, 11),
        std::make_shared<straight>(11, "Track  12", ioposition(1, 3), 200, 10, 0),
    };

    // static std::array<uint8_t, (tracks.size() + 7) / 8> track_mapping = {0};
    // static std::array<uint8_t, (tracks.size() + 7) / 8> readin = {0};

    auto last_track = tracks[0];
    auto current_track = tracks[1];

    while (true)
    {
        auto next_id = current_track->next_track(last_track->id);
        if (next_id < 0 || next_id >= static_cast<int>(tracks.size()))
        {
            MODM_LOG_ERROR << "Invalid track ID: " << current_track->id << " from: " << last_track->id << modm::endl;
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