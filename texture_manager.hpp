#ifndef LECTOR_TEXTURE_MANAGER_HPP
#define LECTOR_TEXTURE_MANAGER_HPP

#include <cstdlib>

namespace lector
{

    class texture_manager
    {
    public:
        texture_manager();
        ~texture_manager();

        void upload(std::size_t width, std::size_t height,
                    void* data)
            const;

        void bind() const;

    private:
        unsigned handle_;
    };

}

#endif
