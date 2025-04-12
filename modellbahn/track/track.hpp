#pragma once
#include <cstddef>
#include "expansion/controller.hpp"
enum class trackid
{
    INVALID = -1,
    A_d,
    A_c,
    A_1a,
    A_1b,
    A_a,
    A_2a,
    A_2b,
    A_b,
    A_3a,
    A_3b,
    B_1a,
    C_a,
    C_1a,
    C_1b,
    C_2a,
    C_2b,
    C_b,
    C_c,
    C_3a,
    C_3b,
    C_3c,
    D_1a,
};

enum class power
{
    OFF,
    ON,
};
enum class track_type
{
    Straight,
    Switch,
    Unknown
};

/// @brief Base class for all track types.
/// @details Provides a common interface for different types of tracks.
struct track
{
    /// @brief The unique identifier of the track.
    const trackid id;


    /// @brief The power position of the track.
    const ioposition power_pos;

    /// @brief The current power state of the track.
    power powerstate;

    /// @brief Constructs a track object.
    /// @param id The unique identifier of the track.
    /// @param power_pos The power position of the track.
    /// @param length The length of the track.
    track(
        trackid id,
        const ioposition &power_pos)
        : id(id),
          power_pos(power_pos),
          powerstate(power::OFF) {}

    /// @brief Virtual destructor for the track class.
    virtual ~track() = default;

    virtual track_type type() const = 0;

    /// @brief Determines the next track based on the previous track.
    /// @param previous The ID of the previous track.
    /// @return The ID of the next track.
    virtual trackid next_track(const trackid previous) const = 0;

    /// @brief Provides a list of possible next tracks.
    /// @param previous The ID of the previous track.
    /// @return An array of up to three possible next track IDs.
    virtual std::array<trackid, 3> next_tracks(const trackid previous) const = 0;

    /// @brief Sets the state of the track to create a path between two tracks.
    /// @param to The ID of the destination track.
    /// @param from The ID of the source track.
    /// @return True if the state was successfully set, false otherwise.
    virtual bool make_way_to([[maybe_unused]] const trackid to, [[maybe_unused]] const trackid from) { return true; };
};
