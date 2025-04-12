#pragma once

#include "track.hpp"

/// @brief Represents a straight track connecting two other tracks.
struct straight : public track
{
    /// @brief Constructs a straight track object.
    /// @param id The unique identifier of the track.
    /// @param power_pos The power position of the track.
    /// @param track_a The first connected track.
    /// @param track_b The second connected track.
    straight(
        trackid id,
        const ioposition &power_pos,
        const trackid track_a,
        const trackid track_b)
        : track(id, power_pos),
          track_a(track_a),
          track_b(track_b) {}

    /// @brief Determines the next track based on the previous track.
    /// @param previous The ID of the previous track.
    /// @return The ID of the next track.
    virtual trackid next_track(const trackid previous) const override
    {
        if (previous == track_a)
        {
            return track_b;
        }
        else if (previous == track_b)
        {
            return track_a;
        }

        return trackid::INVALID;
    }
    virtual track_type type() const override
    {
        return track_type::Straight;
    }

    /// @brief Provides a list of possible next tracks.
    /// @param previous The ID of the previous track.
    /// @return An array of up to three possible next track IDs.
    virtual std::array<trackid, 3> next_tracks(const trackid previous) const override
    {
        if (previous == track_a)
        {
            return {track_b, trackid::INVALID, trackid::INVALID};
        }
        else if (previous == track_b)
        {
            return {track_a, trackid::INVALID, trackid::INVALID};
        }

        return {trackid::INVALID, trackid::INVALID, trackid::INVALID};
    }

private:
    /// @brief The first connected track.
    const trackid track_a;

    /// @brief The second connected track.
    const trackid track_b;
};
