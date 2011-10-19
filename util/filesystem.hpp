#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

namespace filesystem
{

    bool copy_file(const std::string& from_p, const std::string& to_p,
                   bool fail_if_exists = true)
        ;

}

#endif
