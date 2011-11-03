#include "event_handler.cpp"

namespace lector
{

    event_handler::event_handler(::lector::scene& scene)
        : scene_(scene), current_time_(0)
    {}

    void event_handler::set_current_time(std::size_t time)
    {
        current_time_ = time;
    }

    void event_handler::handle_up (std::uint8_t, std::size_t, std::size_t)
    {
        // STUB
    }

    void event_handler::handle_down (std::uint8_t, std::size_t, std::size_t)
    {
        // STUB
    }

    void event_handler::handle_motion (std::uint8_t which, int xrel, int yrel)
    {
        scene.move_by(xrel, yrel);
    }

}
