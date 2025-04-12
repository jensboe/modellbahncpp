#pragma once

#include "track.hpp"

struct straight : public track
{
    straight(size_t power_id, const char *name, size_t length, const size_t track_a, const size_t track_b)
        : track(power_id, name, length), track_a(track_a), track_b(track_b) {}

    virtual size_t next_track(const size_t previous) const override
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
    const size_t track_a;
    const size_t track_b;
};
