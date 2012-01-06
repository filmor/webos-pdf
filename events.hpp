#ifndef LECTOR_EVENTS_HPP
#define LECTOR_EVENTS_HPP

namespace lector
{
namespace events
{
    
    struct tap
    {
        std::size_t x, y;
    };

    struct single_drag
    {
        std::size_t x, y;
        float xrel, yrel;
    };

    struct twin_drag
    {
        single_drag finger1, finger2;
    };

}
}

#endif
