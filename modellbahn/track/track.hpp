#pragma once
#include <cstddef>
#include "expansion/controller.hpp"

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

    const ioposition power_pos;
    power powerstate;

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

    virtual ~track() = default;

    virtual int next_track(const int previous) const = 0;
    virtual std::array<int, 3> next_tracks(const int previous) const = 0;
    virtual bool make_way_to([[maybe_unused]] const int to, [[maybe_unused]] const int from) { return true; };
};
