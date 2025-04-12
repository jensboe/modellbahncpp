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
    switch_track(int id, const char *name, size_t length, const int track_a, const int track_b, const int track_common)
        : track(id, name, length), state(switch_state::UNKNOWN), track_a(track_a), track_b(track_b), track_common(track_common) {}

    virtual int next_track(const int previous) const override
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
        return -1;
    }

private:
    const int track_a;
    const int track_b;
    const int track_common;
};
