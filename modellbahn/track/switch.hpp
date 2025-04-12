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
    switch_track(
        int id,
        const char *name,
        const ioposition &power_pos,
        size_t length,
        const int track_a,
        const int track_b,
        const int track_common,
        const ioposition &straight,
        const ioposition &curved)
        : track(id, name, power_pos, length),
          state(switch_state::UNKNOWN),
          track_a(track_a),
          track_b(track_b),
          track_common(track_common),
          straight(straight),
          curved(curved) {}

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
            return track_common;
        }
        return track_a;
    }
    virtual std::array<int, 3> next_tracks(const int previous) const override
    {
        if (previous == track_common)
        {
            return {track_a, track_b, -1};
        }
        else if (previous == track_a or previous == track_b)
        {
            return {track_common, -1, -1};
        }
        return {-1, -1, -1};
    }
    virtual bool make_way_to(const int to, const int from) override
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
    const int track_a;
    const int track_b;
    const int track_common;

public:
    const ioposition straight;
    const ioposition curved;
};
