#ifndef LECTOR_EVENT_TRANSFORM_HPP
#define LECTOR_EVENT_TRANSFORM_HPP

#ifndef LECTOR_MAX_FINGERS
#define LECTOR_MAX_FINGERS 5
#endif

#include "events.hpp"
#include "scene.hpp"

namespace lector
{

    static const unsigned TAP_TIME = 100; // time in ms

    class event_transform
    {
    public:
        event_transform(lector::scene& scene)
            : scene_(scene), finger_count_(0)
        {}

        void set_current_time(std::size_t current_time)
        {
            current_time_ = current_time;
        }

        void handle_down(int finger, std::size_t x, std::size_t y)
        {
            if (pressed_[finger])
                return;

            pressed_[finger] = true;
            last_pressed_[finger] = current_time_;
            ++finger_count_;
        }

        void handle_up(int finger, std::size_t x, std::size_t y)
        {
            if (pressed_[finger])
            {
                if (
                        (current_time_ - last_pressed_[finger]) < TAP_TIME
                     && (finger_count_ == 1)
                   )
                    scene_.event(events::tap(x, y));
                    ;

                pressed_[finger] = false;
                --finger_count_;
            }
        }

        void handle_motion(int finger, std::size_t x, std::size_t y,
                                        std::size_t xrel, std::size_t yrel)
        {
            if (finger == 0)
            {
                last_x_ = x;
                last_y_ = y;
                scene_.event(events::single_drag(x, y, xrel, yrel));
            }
/*            else if (finger == 1)
            {
                if (pressed_[0])
                    scene_.event(
                            event::twin_drag(last_x_, last_y_, x, y,
                                             xrel, yrel)
                            );
            }*/
        }

    private:
        scene& scene_;
        std::size_t finger_count_;
        std::size_t last_pressed_[LECTOR_MAX_FINGERS];
        bool pressed_[LECTOR_MAX_FINGERS];
        std::size_t last_x_;
        std::size_t last_y_;
        std::size_t current_time_;
    };

}

#endif
