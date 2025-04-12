#pragma once
#include <cstddef>
#include "expansion/controller.hpp"

enum class power
{
    OFF,
    ON,
};

/// @brief Base class for all track types.
/// @details Provides a common interface for different types of tracks.
struct track
{
    /// @brief The unique identifier of the track.
    const int id;

    /// @brief The name of the track.
    const char *name;

    /// @brief The length of the track.
    const size_t length;

    /// @brief The power position of the track.
    const ioposition power_pos;

    /// @brief The current power state of the track.
    power powerstate;

    /// @brief Constructs a track object.
    /// @param id The unique identifier of the track.
    /// @param name The name of the track.
    /// @param power_pos The power position of the track.
    /// @param length The length of the track.
    track(
        int id,
        const char *name,
        const ioposition &power_pos,
        size_t length)
        : id(id),
          name(name),
          length(length),
          power_pos(power_pos),
          powerstate(power::OFF) {}

    /// @brief Virtual destructor for the track class.
    virtual ~track() = default;

    /// @brief Determines the next track based on the previous track.
    /// @param previous The ID of the previous track.
    /// @return The ID of the next track.
    virtual int next_track(const int previous) const = 0;

    /// @brief Provides a list of possible next tracks.
    /// @param previous The ID of the previous track.
    /// @return An array of up to three possible next track IDs.
    virtual std::array<int, 3> next_tracks(const int previous) const = 0;

    /// @brief Sets the state of the track to create a path between two tracks.
    /// @param to The ID of the destination track.
    /// @param from The ID of the source track.
    /// @return True if the state was successfully set, false otherwise.
    virtual bool make_way_to([[maybe_unused]] const int to, [[maybe_unused]] const int from) { return true; };
};
