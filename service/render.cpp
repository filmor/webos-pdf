#include "service.hpp"
#include "../log.hpp"

#include <sys/stat.h>
#include <boost/format.hpp>

namespace lector
{

    static const boost::format render_response ("{\"from\":%d,\"image\":\"%s\"}");
    // <path><prefix><page>-<zoom><suffix>
    static const boost::format filename_format ("%1$s%2$s%5$04d-%3$03d%4$s");

    PDL_bool service::do_render(PDL_JSParameters* params)
    {
        boost::format my_formatter(filename_format);

        int from = PDL_GetJSParamInt(params, 1);
        int count = PDL_GetJSParamInt(params, 2);
        int zoom = PDL_GetJSParamInt(params, 3);
        std::string directory = PDL_GetJSParamString(params, 4);
        std::string prefix = PDL_GetJSParamString(params, 5);
        std::string suffix = PDL_GetJSParamString(params, 6);

        my_formatter % directory % prefix % zoom % suffix;

//        if (!ctx_.is_open())
//            throw std::runtime_error("Document has not been opened yet");

        int err = ::mkdir(directory.c_str(), 0755);
        if (err != 0 && errno != EEXIST)
            throw std::runtime_error("could not create directory");

        LECTOR_LOG("Rendering call: from %d, count %d", from, count);
        for (int i = from; i < from + count; ++i)
        {
            std::string filename = (boost::format(my_formatter) % i).str();

            if (::access(filename.c_str(), R_OK) == -1 && errno == ENOENT)
            {
                LECTOR_LOG("Starting rendering of page %d", i);
                queue_.push(std::make_tuple(zoom / 100.f, i, filename));
            }
            else
            {
                LECTOR_LOG("Reusing cached image of page %d", i);
                std::string response_json =
                    (boost::format(render_response) % i % filename).str();

                const char* response = response_json.c_str();
                PDL_CallJS("RenderCallback", &response, 1);
            }
        }

        return PDL_TRUE;
    }

    void service::render_thread()
    {
        LECTOR_LOG("Started thread");
        context ctx(ctx_);
        LECTOR_LOG("Copied");

        std::tuple<float, int, std::string> elem;

        while(running_)
        {
            LECTOR_LOG("Rendering task");
            queue_.pop(elem);

            float const& zoom = std::get<0>(elem);
            int const& page = std::get<1>(elem);
            std::string const& filename = std::get<2>(elem);
            LECTOR_LOG("Got item");
            try
            {
                pixmap pix = ctx.render_full(zoom, page);
                pix.write_png(filename);
            }
            catch (std::runtime_error const& exc)
            {
                LECTOR_LOG_ERROR("Exception: %s", exc.what());
            }
            LECTOR_LOG("Rendered successfully");

            std::string response_json =
                (boost::format(render_response) % page
                                                % filename).str();

            const char* response = response_json.c_str();
            PDL_CallJS("RenderCallback", &response, 1);
            LECTOR_LOG("Done rendering page %d", page);
        }
    }
    
}
