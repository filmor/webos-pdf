#ifndef LECTOR_SCENE_HPP
#define LECTOR_SCENE_HPP

#include "renderer.hpp"
#include "events.hpp"

namespace lector
{

    class scene
    {
    public:
        scene (renderer&);
        ~scene();

        void draw(std::size_t);

        void event(events::tap const&);
        void event(events::single_drag const&);
        void event(events::twin_drag const&);

    private:
        renderer& renderer_;
        std::size_t current_page_;
        float x_, y_, zoom_level_;
    };

}

#endif
