#ifndef MD5_TO_STRING_HPP
#define MD5_TO_STRING_HPP

#include <string>

// Heavily inspired by Boost.UUID (uuid_io.hpp)
namespace md5
{
    namespace detail
    {
        inline char to_char(std::size_t i) {
            if (i <= 9) {
                return static_cast<char>('0' + i);
            } else {
                return static_cast<char>('a' + (i-10));
            }
        }
    }

    template <typename IteratorT>
    inline std::string md5_to_string(IteratorT begin, IteratorT end)
    {
        std::string result;
        result.reserve(32);

        std::size_t i=0;
        for (IteratorT it_data = begin; it_data!=end; ++it_data, ++i) {
            const size_t hi = ((*it_data) >> 4) & 0x0F;
            result += detail::to_char(hi);

            const size_t lo = (*it_data) & 0x0F;
            result += detail::to_char(lo);
        }
        return result;
    }

}

#endif
