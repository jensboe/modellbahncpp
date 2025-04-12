#pragma once

#include "track.hpp"

enum class switch_state
{
    STRAIGHT,
    CURVED,
    UNKNOWN,
};

/// @brief Represents a switch track with multiple possible states.
/// @details A switch track can connect three tracks and has a state to determine the active connection.
struct switch_track : public track
{
    /// @brief The current state of the switch track.
    switch_state state;

    /// @brief Constructs a switch_track object.
    /// @param id The unique identifier of the track.
    /// @param power_pos The power position of the track.
    /// @param track_a The first connected track.
    /// @param track_b The second connected track.
    /// @param track_common The common track connected to both track_a and track_b.
    /// @param straight The I/O position for the straight state.
    /// @param curved The I/O position for the curved state.
    switch_track(
        trackid id,
        const ioposition &power_pos,
        const trackid track_a,
        const trackid track_b,
        const trackid track_common,
        const ioposition &straight,
        const ioposition &curved)
        : track(id, power_pos),
          state(switch_state::UNKNOWN),
          track_a(track_a),
          track_b(track_b),
          track_common(track_common),
          straight(straight),
          curved(curved) {}
    virtual track_type type() const override
    {
        return track_type::Switch;
    }
    /// @brief Determines the next track based on the current state and the previous track.
    /// @param previous The ID of the previous track.
    /// @return The ID of the next track.
    virtual trackid next_track(const trackid previous) const override
    {
        if (previous == track_common)
        {
            if (state == switch_state::STRAIGHT)
            {
                return track_a;
            }
            else if (state == switch_state::CURVED)
            {
                return track_b;
            }
        }
        else if (previous == track_a or previous == track_b)
        {
            return track_common;
        }
        return trackid::INVALID;
    }

    /// @brief Provides a list of possible next tracks based on the current state.
    /// @param previous The ID of the previous track.
    /// @return An array of up to three possible next track IDs.
    virtual std::array<trackid, 3> next_tracks(const trackid previous) const override
    {
        if (previous == track_common)
        {
            return {
                track_a,
                track_b,
                trackid::INVALID,
            };
        }
        else if (previous == track_a or previous == track_b)
        {
            return {
                track_common,
                trackid::INVALID,
                trackid::INVALID,
            };
        }
        return {trackid::INVALID, trackid::INVALID, trackid::INVALID};
    }

    /// @brief Sets the state of the switch to create a path between two tracks.
    /// @param to The ID of the destination track.
    /// @param from The ID of the source track.
    /// @return True if the state was successfully set, false otherwise.
    virtual bool make_way_to(const trackid to, const trackid from) override
    {
        if (from == track_common)
        {
            if (to == track_a)
            {
                state = switch_state::STRAIGHT;
            }
            else if (to == track_b)
            {
                state = switch_state::CURVED;
            }
            return true;
        }
        else if (to == track_common)
        {
            if (from == track_a)
            {
                state = switch_state::STRAIGHT;
            }
            else if (from == track_b)
            {
                state = switch_state::CURVED;
            }
            return true;
        }
        else
        {
            return false;
        }
    }

private:
    /// @brief The first connected track.
    const trackid track_a;

    /// @brief The second connected track.
    const trackid track_b;

    /// @brief The common track connected to both track_a and track_b.
    const trackid track_common;

public:
    /// @brief The I/O position for the straight state.
    const ioposition straight;

    /// @brief The I/O position for the curved state.
    const ioposition curved;
};
