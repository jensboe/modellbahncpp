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
    switch_track(size_t power_id, const char *name, size_t length)
        : track(power_id, name, length), state(switch_state::UNKNOWN) {}
};
