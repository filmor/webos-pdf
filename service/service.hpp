#ifndef LECTOR_SERVICE_HPP
#define LECTOR_SERVICE_HPP

#include <PDL.h>
#include <boost/thread.hpp>
#include <boost/format.hpp>

#include "../context.hpp"
#include "../util/concurrent_queue.hpp"

namespace lector
{

    class service
    {
    public:
        service(std::size_t, std::size_t);
        ~service();

        void stop()
        {
            running_ = false;
            render_threads_.join_all();
        }

        PDL_bool do_saveas(PDL_JSParameters* params);
        PDL_bool do_find(PDL_JSParameters* params);
        PDL_bool do_open(PDL_JSParameters* params);
        PDL_bool do_toc(PDL_JSParameters* params);
        PDL_bool do_render(PDL_JSParameters* params);

        PDL_bool do_delete(PDL_JSParameters* params) { return PDL_TRUE; }
        PDL_bool do_shell(PDL_JSParameters* params) { return PDL_TRUE; }
        PDL_bool do_cover(PDL_JSParameters* params) { return PDL_TRUE; }

    private:
        void render_thread();

        std::size_t number_of_threads_;

        std::string toc_data_;

        boost::mutex ctx_mutex_;
        boost::thread_group render_threads_;
        context ctx_;

        // zoom, page number, filename-format
        concurrent_queue<std::tuple<float, int, std::string>>
            queue_;

        // TODO: volatile is not really thread-safe
        volatile bool running_;
    };

}

#endif
