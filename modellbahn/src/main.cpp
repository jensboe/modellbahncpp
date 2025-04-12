#include "board.hpp"

#include "track/straight.hpp"
#include "track/switch.hpp"
#include "expansion/controller.hpp"
#include <modm/processing.hpp>
#include <modm/driver/adc/adc_sampler.hpp>

/// @brief Array of tracks in the system.
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

controller<Board::ExpantionBoard::Cs, Board::ExpantionBoard::SpiMaster, 2> expand_control;

modm::Fiber simulation(
    []
    {
        /// @brief Pointer to the last track in the sequence.
        auto last_track = tracks[static_cast<int>(trackid::A_1a)];

        /// @brief Pointer to the current track in the sequence.
        auto current_track = tracks[static_cast<int>(trackid::A_1b)];

        while (true)
        {
            /// @brief Determines the next track based on the current and last tracks.
            auto possible_ways = current_track->next_tracks(last_track->id);
            if (possible_ways[0] == trackid::INVALID)
            {
                MODM_LOG_ERROR << "Next track not found " << static_cast<int>(current_track->id) << " comming from " << static_cast<int>(last_track->id) << modm::endl;
                break;
            }
            else
            {
                auto select = possible_ways[0];
                if (possible_ways[1] != trackid::INVALID)
                {
                    // MODM_LOG_INFO << "Select: " << static_cast<int>(select) << "\tPossible: " << static_cast<int>(possible_ways[1]) << modm::endl;
                    if (Board::Nucleo::Button::read())
                    {
                        select = possible_ways[1];
                    }
                }

                current_track->make_way_to(select, last_track->id);
            }

            auto next_id = current_track->next_track(last_track->id);

            auto next_track = tracks[static_cast<int>(next_id)];

            MODM_LOG_INFO << "Current: " << static_cast<int>(current_track->id)
                          << "\tNext:  " << static_cast<int>(next_track->id)
                          << "\tLast:  " << static_cast<int>(last_track->id) << modm::endl;

            last_track = current_track;
            current_track = next_track;
            last_track->powerstate = power::OFF;
            current_track->powerstate = power::ON;

            /// @brief Updates the power state of the tracks.
            for (const auto &track : tracks)
            {
                expand_control.set_buffer(track->power_pos, track->powerstate == power::ON);
                if (track->type() == track_type::Switch)
                {
                    auto handle_switch = static_cast<switch_track *>(track.get());
                    if (handle_switch->state == switch_state::STRAIGHT)
                    {
                        expand_control.set_buffer(handle_switch->straight, true);
                        expand_control.set_buffer(handle_switch->curved, false);
                    }
                    else if (handle_switch->state == switch_state::CURVED)
                    {
                        expand_control.set_buffer(handle_switch->straight, false);
                        expand_control.set_buffer(handle_switch->curved, true);
                    }
                }
            }
            Board::Nucleo::LedBlue::toggle();
            modm::this_fiber::sleep_for(100ms);
        }
    });
modm::Fiber measurement(
    []
    {
        typedef GpioInputA3 AdcCurrent;
        typedef GpioInputC0 AdcVoltage;
        // AdcVoltage::In10
        // AdcCurrent::In3
        Adc1::Channel sensorMapping[3] = {
            Adc1::getPinChannel<AdcCurrent>(),
            Adc1::getPinChannel<AdcVoltage>(),
            Adc1::Channel::TemperatureSensor,
        };
        uint32_t sensorData[3];

        typedef modm::AdcSampler<AdcInterrupt1, 3, 100> sensors;

        Adc1::connect<AdcCurrent::In3, AdcVoltage::In10>();
        Adc1::initialize<Board::SystemClock, 11'250_kHz>();

        Adc1::enableInterruptVector(5);
        Adc1::enableInterrupt(Adc1::Interrupt::EndOfRegularConversion);

        sensors::initialize(sensorMapping, sensorData);

        while (true)
        {
            sensors::startReadout();
            modm::this_fiber::poll(sensors::isReadoutFinished);
            uint32_t *data = sensors::getData();

            MODM_LOG_INFO << "current=" << data[0];
            MODM_LOG_INFO << "\tvoltagey=" << data[1];;
            MODM_LOG_INFO << "\ttemperature=" << data[2] << modm::endl;
            modm::this_fiber::sleep_for(100ms);
        }
    });
int main()
{
    Board::initialize();

    Board::Adapter_A::LedRed::set(true);
    modm::delay(500ms);
    Board::Adapter_A::LedRed::set(false);
    Board::Adapter_A::LedYellow::set(true);
    modm::delay(200ms);
    Board::Adapter_A::LedYellow::set(false);
    Board::Adapter_A::LedGreen::set(true);

    modm::fiber::Scheduler::run();
    return 0;
}
