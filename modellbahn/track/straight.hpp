#pragma once

#include "track.hpp"

struct straight : public track
{
    straight(int id, const char *name, const ioposition power_pos, size_t length, const int track_a, const int track_b)
        : track(id, name, power_pos, length), track_a(track_a), track_b(track_b) {}

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

private:
    const int track_a;
    const int track_b;
};
