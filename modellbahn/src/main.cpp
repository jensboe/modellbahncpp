#include "board.hpp"

#include "track/straight.hpp"
#include "track/switch.hpp"

int main()
{
    Board::initialize();

    Board::Adapter_A::LedRed::set(true);
    modm::delay(100ms);
    Board::Adapter_A::LedRed::set(false);
    Board::Adapter_A::LedYellow::set(true);
    modm::delay(10ms);
    Board::Adapter_A::LedYellow::set(false);
    Board::Adapter_A::LedGreen::set(true);

    static const std::array<std::shared_ptr<track>, 11> tracks = {
        std::make_shared<straight>(0, "Track  1", 200, tracks.size() - 1, 1),
        std::make_shared<straight>(1, "Track  2", 200, 0, 2),
        std::make_shared<straight>(2, "Track  3", 200, 1, 3),
        std::make_shared<straight>(3, "Track  4", 200, 2, 4),
        std::make_shared<straight>(4, "Track  5", 200, 3, 5),
        std::make_shared<straight>(5, "Track  6", 200, 4, 6),
        std::make_shared<straight>(6, "Track  7", 200, 5, 7),
        std::make_shared<straight>(7, "Track  8", 200, 6, 8),
        std::make_shared<straight>(8, "Track  9", 200, 7, 9),
        std::make_shared<straight>(9, "Track 10", 200, 8, 10),
        std::make_shared<straight>(10, "Track 11", 200, 9, 0),
    };

    static std::array<uint8_t, (tracks.size() + 7) / 8> track_mapping = {0};
    static std::array<uint8_t, (tracks.size() + 7) / 8> readin = {0};

    auto last_track = tracks[0];
    auto current_track = tracks[1];

    while (true)
    {
        Board::Nucleo::LedBlue::toggle();
        modm::delay(50ms);
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
            // MODM_LOG_INFO << "Track ID: " << track->power_id
            //               << ", Name: " << track->name
            //               << ", Power State: " << (track->powerstate == power::ON ? "ON" : "OFF")
            //               << modm::endl;
            if (track->powerstate == power::ON)
            {
                track_mapping[track->id / 8] |= static_cast<uint8_t>(1 << (track->id % 8));
            }
            else
            {
                track_mapping[track->id / 8] &= static_cast<uint8_t>(~(1 << (track->id % 8)));
            }
        }
        Board::ExpantionBoard::Cs::set(false);
        Board::ExpantionBoard::SpiMaster::transferBlocking(track_mapping.data(), readin.data(), track_mapping.size());
        Board::ExpantionBoard::Cs::set(true);
    }
    MODM_LOG_ERROR << "Abandoning...\n"
                   << modm::flush;
    while (true)
    {
    }
    return 0;
}