#pragma once

#include "track.hpp"

/// @brief Represents a straight track connecting two other tracks.
struct straight : public track
{
    /// @brief Constructs a straight track object.
    /// @param id The unique identifier of the track.
    /// @param name The name of the track.
    /// @param power_pos The power position of the track.
    /// @param length The length of the track.
    /// @param track_a The first connected track.
    /// @param track_b The second connected track.
    straight(
        int id,
        const char *name,
        const ioposition &power_pos,
        size_t length,
        const int track_a,
        const int track_b)
        : track(id, name, power_pos, length),
          track_a(track_a),
          track_b(track_b) {}

    /// @brief Determines the next track based on the previous track.
    /// @param previous The ID of the previous track.
    /// @return The ID of the next track.
    virtual int next_track(const int previous) const override
    {
        if (previous == track_a)
        {
            return track_b;
        }
        else if (previous == track_b)
        {
            return track_a;
        }

        return -1;
    }

    /// @brief Provides a list of possible next tracks.
    /// @param previous The ID of the previous track.
    /// @return An array of up to three possible next track IDs.
    virtual std::array<int, 3> next_tracks(const int previous) const override
    {
        if (previous == track_a)
        {
            return {track_b, -1, -1};
        }
        else if (previous == track_b)
        {
            return {track_a, -1, -1};
        }

        return {-1, -1, -1};
    }

private:
    /// @brief The first connected track.
    const int track_a;

    /// @brief The second connected track.
    const int track_b;
};
