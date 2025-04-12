#pragma once

#include "track.hpp"

enum class switch_state
{
    STRAIGHT,
    CURVED,
    UNKNOWN,
};

struct switch_track : public track
{
    switch_state state;
    switch_track(size_t power_id, const char *name, size_t length, const size_t track_a, const size_t track_b, const size_t track_common)
        : track(power_id, name, length), state(switch_state::UNKNOWN), track_a(track_a), track_b(track_b), track_common(track_common) {}

    virtual size_t next_track(const size_t previous) const override
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
            if (state == switch_state::STRAIGHT)
            {
                return track_common;
            }
        }
        return 10000;
    }

private:
    const size_t track_a;
    const size_t track_b;
    const size_t track_common;
};
