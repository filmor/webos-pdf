#include "service.hpp"

#include "../log.hpp"
#include "../util/md5_to_string.hpp"

#include <fstream>

namespace lector
{
    static const boost::format open_response
        ("{\"pages\":%d,\"width\":%d,\"height\":%d,\"units\":\"pt\""
         ",\"dst\":\"/media/internal/appdata/com.quickoffice.ar/.cache/%s\"}");

    static const boost::format toc_element ("{\"p\":%d,\"l\":%d,\"c\":\"%s\"},");
    static const boost::format error ("{\"error\":\"%s\"");

    // p: Page
    // l: Level
    // c: Content (aka Name)
    namespace
    {
        void print_outline(std::string& str, fz_outline const* out, unsigned level)
        {
            while (out)
            {
                if (out->dest.kind == FZ_LINK_GOTO)
                {
                    const std::size_t page = out->dest.ld.gotor.page;

                    str += (boost::format(toc_element) % page
                                                       % level
                                                       % out->title).str();
                }

                if (out->down)
                    print_outline(str, out->down, level + 1);            
                out = out->next;
            }
        }
    }

    PDL_bool service::do_open(PDL_JSParameters* params)
    {
        std::string r = "";

        try
        {
            std::string filename = PDL_GetJSParamString(params, 1);

            LECTOR_LOG("Loading file");
            ctx_.load_file(filename);
            LECTOR_LOG("Loaded file");

            if (ctx_.needs_password())
            {
                std::string password = PDL_GetJSParamString(params, 2);
                if (!ctx_.authenticate(password))
                    throw pdf_exception("PASSWORD WRONG MESSAGE");
            }

            // Generate Unique ID as an md5 of the first kilobyte of the file
            std::ifstream file (filename.c_str());
            
            fz_md5 md5;
            fz_md5_init(&md5);
            unsigned char buffer[1024];
            file.read((char*)buffer, 1024);
            fz_md5_update(&md5, buffer, file.gcount());
            file.close();

            // Reuse buffer
            fz_md5_final(&md5, buffer);

            // Cache outline string, it will be used directly afterwards.
            print_outline(toc_data_, ctx_.get_outline(), 0);

            fz_rect bbox = ctx_.get_bbox(0);

            // Generate hex output
            std::string digest = md5::md5_to_string(buffer + 0, buffer + 16);

            r = (boost::format(open_response) % ctx_.get_page_count()
                                              % (bbox.x1 - bbox.x0)
                                              % (bbox.y1 - bbox.y0)
                                              % digest).str();
        }
        catch (std::exception const& exc)
        {
            r = (boost::format(error) % exc.what()).str();
        }

        for (unsigned i = 0; i < number_of_threads_; ++i)
            render_threads_.add_thread(
                    new boost::thread(boost::bind(&service::render_thread, this)
                        ));

        LECTOR_LOG("Started threads");

        const char* ptr = r.c_str();

        PDL_CallJS("OpenCallback", &ptr, 1);
        return PDL_TRUE;
    }
    
    static const boost::format toc_response ("{\"toc\":[%s]}");

    PDL_bool service::do_toc(PDL_JSParameters* params)
    {
        std::string const& result = (boost::format(toc_response) % toc_data_).str();
        const char* ptr = result.c_str();
        PDL_CallJS("TocCallback", &ptr, 1);

        return PDL_TRUE;
    }
    
}

