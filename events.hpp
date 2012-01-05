#ifndef LECTOR_EVENTS_HPP
#define LECTOR_EVENTS_HPP

namespace lector
{
namespace events
{
    
    struct tap
    {
        tap (std::size_t x_, std::size_t y_)
            : x(x_), y(y_)
        {}

        std::size_t x, y;
    };

    struct single_drag
    {
        single_drag (std::size_t x_, std::size_t y_, float xrel_, float yrel_)
            : x(x_), y(y_), xrel(xrel_), yrel(yrel_)
        {}

        std::size_t x, y;
        float xrel, yrel;
    };

    struct twin_drag
    {
    };

}
}

#endif
