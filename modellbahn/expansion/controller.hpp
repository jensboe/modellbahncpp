#pragma once
#include <cstdint>

struct ioposition
{
    size_t board;
    uint8_t bit_pos;
};
struct board
{
    const size_t inputs;
    const size_t outputs;
    constexpr size_t buffer_size() const
    {
        return std::max(inputs, outputs);
    }
};
static constexpr board board_a = {
    .inputs = 0,
    .outputs = 1,
};
static constexpr std::array<board, 7> boards = {
    board_a,
    board_a,
    board_a,
    board_a,
    board_a,
    board_a,
    board_a,
};

static constexpr size_t calculate_buffer_size()
{
    size_t size = 0;
    for (const auto &b : boards)
    {
        size += b.buffer_size();
    }
    return size;
}
static constexpr size_t calculate_buffer_offset(size_t board_index)
{
    size_t offset = 0;

    for (size_t i = boards.size() - 1; i >= board_index; --i)
    {
        if (i <= board_index)
        {
            break;
        }
        offset += boards[i].buffer_size();
    }
    return offset;
}

struct controller
{
    static constexpr size_t buffer_size = calculate_buffer_size();

    std::array<uint8_t, buffer_size> out_buffer = {0};
    std::array<uint8_t, buffer_size> in_buffer = {0};

    void set_buffer(const ioposition &pos, bool state)
    {
        if (pos.board < boards.size())
        {
            if (pos.bit_pos / 8 < boards[pos.board].outputs)
            {
                auto buffer_index = calculate_buffer_offset(pos.board) + pos.bit_pos / 8;
                if (state)
                {
                    out_buffer[buffer_index] |= static_cast<uint8_t>((1 << (pos.bit_pos % 8)));
                }
                else
                {
                    out_buffer[buffer_index] &= static_cast<uint8_t>(~(1 << (pos.bit_pos % 8)));
                }
            }
            else
            {
                MODM_LOG_ERROR << "Invalid bit position: " << pos.bit_pos << " for board: " << pos.board << modm::endl;
            }
        }
        else
        {
            MODM_LOG_ERROR << "Invalid board position: " << pos.board << modm::endl;
        }
    }
    void update()
    {
        Board::ExpantionBoard::Cs::set(false);
        Board::ExpantionBoard::SpiMaster::transferBlocking(out_buffer.data(), in_buffer.data(), buffer_size);
        Board::ExpantionBoard::Cs::set(true);
    }
};
