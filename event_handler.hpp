#ifndef LECTOR_SDL_EVENT_HANDLER_HPP
#define LECTOR_SDL_EVENT_HANDLER_HPP

namespace lector
{

    // Handles raw mouse events and emits gesture-events from them
    class event_handler
    {
    public:
        event_handler(lector::scene& scene);

        void set_current_time(std::size_t time);
        void handle_up(uint8_t index, std::size_t x, std::size_t y);
        void handle_down(uint8_t index, std::size_t x, std::size_t y);
        void handle_motion(uint8_t index, int xrel, int yrel);

    private:
        std::size_t time_;
        lector::scene& scene_;
    };

}

#endif
