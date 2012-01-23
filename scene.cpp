#include "scene.hpp"

namespace lector
{

    scene::scene (renderer& r) :
        renderer_(r), current_page_(0), x_(0), y_(0),
        zoom_level_(0.f)
    {
    }

    scene::~scene() {}

    void scene::draw(std::size_t)
    {
        renderer_.render_page(current_page_, x_, y_, zoom_level_);
    }

    void scene::event(events::tap const&)
    {
        current_page_ = (current_page_ + 1) % renderer_.get_page_count();
        renderer_.switch_to_page(current_page_);
        // renderer_.cache(current_page_, 1);
    }

    void scene::event(events::single_drag const& evt)
    {
        x_ += evt.xrel;
        y_ += evt.yrel;
    }

    void scene::event(events::twin_drag const& evt)
    {
    }
}

