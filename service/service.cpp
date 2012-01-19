#include "service.hpp"

namespace lector
{

    service::service(std::size_t storage_limit, std::size_t threads)
        : number_of_threads_(threads), ctx_(storage_limit), running_(true)
    {
        fz_accelerate();
        ctx_.call(fz_set_aa_level, 8);
    }

    service::~service()
    {
        stop();
    }

}

