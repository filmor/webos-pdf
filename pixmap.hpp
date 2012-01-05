#ifndef LECTOR_PIXMAP_HPP
#define LECTOR_PIXMAP_HPP

extern "C"
{
#include <fitz.h>
}

namespace lector
{

    // Thin wrapper around fz_pixmap
    class pixmap
    {
    public:
        pixmap(fz_context* ctx, fz_bbox const& bbox,
               fz_colorspace* colorspace = fz_device_rgb)
            : ctx_(ctx),
              pix_(fz_new_pixmap_with_rect(ctx, colorspace, bbox))
        {}

        pixmap(pixmap const& pix)
            : ctx_(pix.ctx_),
              pix_(fz_keep_pixmap(pix.pix_))
        {}

        ~pixmap()
        {
            fz_drop_pixmap(ctx_, pix_);
        }

        void clear(std::size_t color = 0xFF)
        {
            if (color == 0)
                fz_clear_pixmap(pix_);

            fz_clear_pixmap_with_color(pix_, color);
        }

        void write_png(std::string const& str) const
        {
            // TODO: Error handling, Alpha channel
            fz_write_png(ctx_,
                         const_cast<fz_pixmap*> (pix_),
                         const_cast<char*> (str.c_str()),
                         0);
        }

        void* get_data() { return pix_->samples; }

        // TODO: This should go away!
        fz_pixmap* get() { return pix_; }

        std::size_t width() const { return pix_->w; }
        std::size_t height() const { return pix_->h; }

    private:
        fz_context* ctx_;
        fz_pixmap* pix_;
    };

}

#endif
