#include "service.hpp"

#include <syslog.h>
#include <sys/stat.h>
#include <boost/format.hpp>

namespace lector
{

    static const boost::format render_response ("{\"from\":%d,\"image\":\"%s\"}");
    // <path><prefix><page>-<zoom><suffix>
    static const boost::format filename_format ("%1$s%2$s%5$04d-%3$03d%4$s");

    PDL_bool service::do_render(PDL_JSParameters* params)
    {
        syslog(LOG_INFO, "Called do_render");
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

        syslog(LOG_INFO, "Rendering call");
        for (int i = from; i < from + count; ++i)
        {
            std::string filename = (boost::format(filename_format) % i).str();

            if (::access(filename.c_str(), R_OK) == -1 && errno == ENOENT)
            {
                syslog(LOG_INFO, "Starting rendering of page %d", i);
                queue_.push(std::make_tuple(zoom / 100.f, i, filename));
            }
            else
            {
                syslog(LOG_INFO, "Reusing cached image of page %d", i);
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
        syslog(LOG_INFO, "Started thread");
        context ctx(ctx_);
        syslog(LOG_INFO, "Copied");

        std::tuple<float, int, std::string> elem;

        while(running_)
        {
            syslog(LOG_INFO, "Rendering task");
            float const& zoom = std::get<0>(elem);
            int const& page = std::get<1>(elem);
            std::string const& filename = std::get<2>(elem);
            queue_.pop(elem);
            syslog(LOG_INFO, "Got item");
            ctx.render_full(zoom, page).write_png(filename);

            std::string response_json =
                (boost::format(render_response) % page
                                                % filename).str();

            const char* response = response_json.c_str();
            PDL_CallJS("RenderCallback", &response, 1);
            syslog(LOG_INFO, "Done rendering page %d", page);
        }
    }
    
}
