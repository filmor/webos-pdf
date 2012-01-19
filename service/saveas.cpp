#include "service.hpp"

#include "../util/filesystem.hpp"

namespace lector
{
    PDL_bool service::do_saveas(PDL_JSParameters* params)
    {
        static const boost::format response_fmt("{\"result\":%d}");

        std::string src = PDL_GetJSParamString(params, 2);
        std::string dst = PDL_GetJSParamString(params, 3);
        bool overwrite = PDL_GetJSParamInt(params, 4);

        int result = filesystem::copy_file(src, "/media/internal/" + dst, overwrite);
        std::string response = (boost::format(response_fmt) % result).str();
        
        const char* ptr = response.c_str();
        PDL_CallJS("SaveAsCallback", &ptr, 1);
        return PDL_TRUE;
    }
}
    
