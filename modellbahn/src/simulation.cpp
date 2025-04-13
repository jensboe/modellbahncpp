#include <modm/processing.hpp>
#include "track/layout.hpp"
#include "board.hpp"

controller<Board::ExpantionBoard::Cs, Board::ExpantionBoard::SpiMaster, 2> expand_control;

modm::Fiber simulation(
    []
    {
        /// @brief Pointer to the last track in the sequence.
        auto last_track = tracks[static_cast<int>(trackid::A_1a)];

        /// @brief Pointer to the current track in the sequence.
        auto current_track = tracks[static_cast<int>(trackid::A_1b)];

        while (true)
        {
            /// @brief Determines the next track based on the current and last tracks.
            auto possible_ways = current_track->next_tracks(last_track->id);
            if (possible_ways[0] == trackid::INVALID)
            {
                MODM_LOG_ERROR << "Next track not found " << static_cast<int>(current_track->id) << " comming from " << static_cast<int>(last_track->id) << modm::endl;
                break;
            }
            else
            {
                auto select = possible_ways[0];
                if (possible_ways[1] != trackid::INVALID)
                {
                    // MODM_LOG_INFO << "Select: " << static_cast<int>(select) << "\tPossible: " << static_cast<int>(possible_ways[1]) << modm::endl;
                    if (Board::Nucleo::Button::read())
                    {
                        select = possible_ways[1];
                    }
                }

                current_track->make_way_to(select, last_track->id);
            }

            auto next_id = current_track->next_track(last_track->id);

            auto next_track = tracks[static_cast<int>(next_id)];

            MODM_LOG_INFO << "Current: " << static_cast<int>(current_track->id)
                          << "\tNext:  " << static_cast<int>(next_track->id)
                          << "\tLast:  " << static_cast<int>(last_track->id) << modm::endl;

            last_track = current_track;
            current_track = next_track;
            last_track->powerstate = power::OFF;
            current_track->powerstate = power::ON;

            /// @brief Updates the power state of the tracks.
            for (const auto &track : tracks)
            {
                expand_control.set_buffer(track->power_pos, track->powerstate == power::ON);
                if (track->type() == track_type::Switch)
                {
                    auto handle_switch = static_cast<switch_track *>(track.get());
                    if (handle_switch->state == switch_state::STRAIGHT)
                    {
                        expand_control.set_buffer(handle_switch->straight, true);
                        expand_control.set_buffer(handle_switch->curved, false);
                    }
                    else if (handle_switch->state == switch_state::CURVED)
                    {
                        expand_control.set_buffer(handle_switch->straight, false);
                        expand_control.set_buffer(handle_switch->curved, true);
                    }
                }
            }
            Board::Nucleo::LedBlue::toggle();
            modm::this_fiber::sleep_for(100ms);
        }
    });