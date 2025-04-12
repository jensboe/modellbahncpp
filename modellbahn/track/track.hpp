#pragma once
#include <cstddef>

enum class power
{
    OFF,
    ON,
};

struct track
{
    const size_t power_id;
    const char *name;
    const size_t length;

    power powerstate;
    track(size_t power_id, const char *name, size_t length)
        : power_id(power_id), name(name), length(length), powerstate(power::OFF) {}

    virtual ~track() = default;

    virtual size_t next_track(const size_t previous) const = 0;
};
