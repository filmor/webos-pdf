#ifndef PIXMAP_HPP
#define PIXMAP_HPP

extern "C"
{
#include <fitz.h>
}

namespace viewer
{

    // Thin wrapper around fz_pixmap
    class pixmap
    {
    public:
        pixmap(std::size_t width, std::size_t height,
               fz_colorspace* colorspace = fz_device_rgb)
            : pix_(fz_new_pixmap(colorspace, width, height))
        {}

        pixmap(pixmap const& pix)
            : pix_(fz_keep_pixmap(pix.pix_))
        {}

        ~pixmap()
        {
            fz_drop_pixmap(pix_);
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
            fz_write_png(const_cast<fz_pixmap*> (pix_),
                         const_cast<char*> (str.c_str()),
                         0);
        }

        void* get_data() { return pix_->samples; }

        // TODO: This should go away!
        fz_pixmap* get() { return pix_; }

    private:
        fz_pixmap* pix_;
    };

}

#endif
