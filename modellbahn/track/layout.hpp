#pragma once
#include <memory>
#include <array>
#include "straight.hpp"
#include "switch.hpp"
#include "track.hpp"

static const auto tracks = std::to_array<std::shared_ptr<track>>({
    std::make_shared<switch_track>(trackid::A_d, ioposition(0, 1), trackid::A_c, trackid::A_3a, trackid::D_1a, ioposition(1, 3), ioposition(1, 4)),
    std::make_shared<switch_track>(trackid::A_c, ioposition(0, 2), trackid::A_1a, trackid::A_2a, trackid::A_d, ioposition(1, 5), ioposition(1, 6)),
    std::make_shared<straight>(trackid::A_1a, ioposition(0, 3), trackid::A_c, trackid::A_1b),
    std::make_shared<straight>(trackid::A_1b, ioposition(0, 4), trackid::A_1a, trackid::A_a),
    std::make_shared<switch_track>(trackid::A_a, ioposition(0, 5), trackid::A_1b, trackid::A_b, trackid::B_1a, ioposition(1, 7), ioposition(2, 0)),
    std::make_shared<straight>(trackid::A_2a, ioposition(0, 6), trackid::A_c, trackid::A_2b),
    std::make_shared<straight>(trackid::A_2b, ioposition(0, 7), trackid::A_2a, trackid::A_b),
    std::make_shared<switch_track>(trackid::A_b, ioposition(1, 0), trackid::A_3b, trackid::A_2b, trackid::A_a, ioposition(2, 1), ioposition(2, 2)),
    std::make_shared<straight>(trackid::A_3a, ioposition(1, 1), trackid::A_d, trackid::A_3b),
    std::make_shared<straight>(trackid::A_3b, ioposition(1, 2), trackid::A_3a, trackid::A_b),

    std::make_shared<straight>(trackid::B_1a, ioposition(3, 0), trackid::A_a, trackid::C_a),
    std::make_shared<switch_track>(trackid::C_a, ioposition(4, 0), trackid::C_2a, trackid::C_1a, trackid::B_1a, ioposition(5, 2), ioposition(5, 3)),
    std::make_shared<straight>(trackid::C_1a, ioposition(4, 1), trackid::C_a, trackid::C_1b),
    std::make_shared<straight>(trackid::C_1b, ioposition(4, 2), trackid::C_1a, trackid::C_b),
    std::make_shared<straight>(trackid::C_2a, ioposition(4, 3), trackid::C_a, trackid::C_2b),
    std::make_shared<straight>(trackid::C_2b, ioposition(4, 4), trackid::C_2a, trackid::C_b),
    std::make_shared<switch_track>(trackid::C_b, ioposition(4, 5), trackid::C_1b, trackid::C_2b, trackid::C_c, ioposition(5, 4), ioposition(5, 5)),
    std::make_shared<switch_track>(trackid::C_c, ioposition(4, 6), trackid::C_b, trackid::C_3c, trackid::D_1a, ioposition(5, 6), ioposition(5, 7)),
    std::make_shared<straight>(trackid::C_3a, ioposition(4, 7), trackid::C_3b, trackid::C_3b), // create type "end"
    std::make_shared<straight>(trackid::C_3b, ioposition(5, 0), trackid::C_3a, trackid::C_3c),
    std::make_shared<straight>(trackid::C_3c, ioposition(5, 1), trackid::C_3b, trackid::C_c),

    std::make_shared<straight>(trackid::D_1a, ioposition(0, 0), trackid::C_c, trackid::A_d),
});