#pragma once
#include <cstddef>

enum class power
{
    OFF,
    ON,
};

struct track
{
    const int id;
    const char *name;
    const size_t length;

    power powerstate;
    track(int id, const char *name, size_t length)
        : id(id), name(name), length(length), powerstate(power::OFF) {}

    virtual ~track() = default;

    virtual int next_track(const int previous) const = 0;
};
