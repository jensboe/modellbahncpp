#include "board.hpp"

#include "track/track.hpp"

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

    std::array<track, 20> tracks = {{
        {0, "Track 1", 100},
        {1, "Track 2", 300},
        {2, "Track 3", 200},
        {3, "Track 4", 400},
        {4, "Track 5", 500},
        {5, "Track 6", 600},
        {6, "Track 7", 700},
        {7, "Track 8", 800},
        {8, "Track 9", 900},
        {9, "Track 10", 1000},
        {10, "Track 11", 1100},
        {11, "Track 12", 1200},
        {12, "Track 13", 1300},
        {13, "Track 14", 1400},
        {14, "Track 15", 1500},
        {15, "Track 16", 1600},
        {16, "Track 17", 1700},
        {17, "Track 18", 1800},
        {18, "Track 19", 1900},
        {19, "Track 20", 2000},
    }};

    static std::array<uint8_t, (tracks.size() + 7) / 8> track_mapping = {0};
    static std::array<uint8_t, (tracks.size() + 7) / 8> readin = {0};

    while (true)
    {
        Board::Nucleo::LedBlue::toggle();
        static uint32_t counter(0);
        modm::delay(50ms);
        MODM_LOG_INFO << "Loop counter: " << (counter++) << modm::endl;

        if (counter % 30 == 0)
        {
            tracks[0].powerstate = tracks[0].powerstate == power::ON ? power::OFF : power::ON;
        }
        if (counter % 70 == 0)
        {
            tracks[1].powerstate = tracks[1].powerstate == power::ON ? power::OFF : power::ON;
        }
        if (counter % 110 == 0)
        {
            tracks[2].powerstate = tracks[2].powerstate == power::ON ? power::OFF : power::ON;
        }
        for (const auto &track : tracks)
        {
            MODM_LOG_INFO << "Track ID: " << track.power_id
                          << ", Name: " << track.name
                          << ", Power State: " << (track.powerstate == power::ON ? "ON" : "OFF")
                          << ", Length: " << track.length
                          << modm::endl;
            if (track.powerstate == power::ON)
            {
                track_mapping[track.power_id / 8] |= (1 << (track.power_id % 8));
            }
            else
            {
                track_mapping[track.power_id / 8] &= static_cast<uint8_t>(~(1 << (track.power_id % 8)));
            }
        }
        Board::ExpantionBoard::Cs::set(false);
        Board::ExpantionBoard::SpiMaster::transferBlocking(track_mapping.data(), readin.data(), track_mapping.size());
        Board::ExpantionBoard::Cs::set(true);
    }
    return 0;
}