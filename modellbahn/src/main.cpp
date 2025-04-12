#include "board.hpp"

#include "track/straight.hpp"
#include "track/switch.hpp"
#include "expansion/controller.hpp"

/// @brief Entry point of the program.
/// @details Initializes the board and controls the track system in a loop.
int main()
{
    /// @brief Initializes the board hardware.
    Board::initialize();
    controller controller;

    Board::Adapter_A::LedRed::set(true);
    modm::delay(100ms);
    Board::Adapter_A::LedRed::set(false);
    Board::Adapter_A::LedYellow::set(true);
    modm::delay(10ms);
    Board::Adapter_A::LedYellow::set(false);
    Board::Adapter_A::LedGreen::set(true);

    /// @brief Array of tracks in the system.
    static const auto tracks = std::to_array<std::shared_ptr<track>>({
        std::make_shared<straight>(0, "Track  1", ioposition(0, 0), 200, 4, 1),
        std::make_shared<switch_track>(1, "Switch 1", ioposition(0, 1), 200, 2, 3, 0, ioposition(0, 5), ioposition(0, 6)),
        std::make_shared<straight>(2, "Track  2a", ioposition(0, 2), 200, 1, 4),
        std::make_shared<straight>(3, "Track  2b", ioposition(0, 3), 200, 1, 4),
        std::make_shared<switch_track>(4, "Switch 2", ioposition(0, 4), 200, 2, 3, 0, ioposition(0, 7), ioposition(1, 0)),
    });

    /// @brief Pointer to the last track in the sequence.
    auto last_track = tracks[0];

    /// @brief Pointer to the current track in the sequence.
    auto current_track = tracks[1];

    while (true)
    {
        /// @brief Determines the next track based on the current and last tracks.
        auto possible_ways = current_track->next_tracks(last_track->id);
        if (possible_ways[0] < 0 || possible_ways[0] >= static_cast<int>(tracks.size()))
        {
            MODM_LOG_ERROR << "Next track not found " << current_track->name << " comming from  " << last_track->name << modm::endl;
            break;
        }
        else
        {
            auto select = possible_ways[0];
            if (possible_ways[1] != -1)
            {
                if (Board::Nucleo::Button::read())
                {
                    select = possible_ways[1];
                }
            }

            current_track->make_way_to(select, last_track->id);
        }

        auto next_id = current_track->next_track(last_track->id);

        auto next_track = tracks[next_id];

        MODM_LOG_INFO << "Current: " << current_track->name << "\tNext: " << next_track->name << "\tLast: " << last_track->name << modm::endl;

        last_track = current_track;
        current_track = next_track;
        last_track->powerstate = power::OFF;
        current_track->powerstate = power::ON;

        /// @brief Updates the power state of the tracks.
        for (const auto &track : tracks)
        {
            controller.set_buffer(track->power_pos, track->powerstate == power::ON);
            if (track->type() == track_type::Switch)
            {
                auto handle_switch = static_cast<switch_track *>(track.get());
                if (handle_switch->state == switch_state::STRAIGHT)
                {
                    controller.set_buffer(handle_switch->straight, true);
                    controller.set_buffer(handle_switch->curved, false);
                }
                else if (handle_switch->state == switch_state::CURVED)
                {
                    controller.set_buffer(handle_switch->straight, false);
                    controller.set_buffer(handle_switch->curved, true);
                }
            }
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