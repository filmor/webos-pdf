#ifndef LECTOR_TILE_MANAGER_HPP
#define LECTOR_TILE_MANAGER_HPP

#include "context.hpp"

#include <vector>

namespace lector
{

    class tile_manager
    {
    public:
        tile_manager(context& ctx);
        ~tile_manager();

        void bind() const;

        void generate();

        template <typename Func>
        void register_callback(Func func);

    private:
        struct node;

        std::vector<std::vector<bool>> page_caches_;

        context& ctx_;
        unsigned handle_;
    };

}

#endif
