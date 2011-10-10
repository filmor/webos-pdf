#include <boost/scoped_array.hpp>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

// Shamelessly stolen from Boost.Filesystem V3
// (c) 2001 Dietmar Kuehl
// (c) 2002-2009 Beman Dawes
namespace filesystem
{

bool // true if ok
  copy_file(const std::string& from_p,
    const std::string& to_p, bool fail_if_exists)
  {
    const std::size_t buf_sz = 32768;
    boost::scoped_array<char> buf(new char [buf_sz]);
    int infile=-1, outfile=-1;  // -1 means not open

    // bug fixed: code previously did a stat()on the from_file first, but that
    // introduced a gratuitous race condition; the stat()is now done after the open()

    if ((infile = ::open(from_p.c_str(), O_RDONLY))< 0)
      { return false; }

    struct stat from_stat;
    if (::stat(from_p.c_str(), &from_stat)!= 0)
    { 
      ::close(infile);
      return false;
    }

    int oflag = O_CREAT | O_WRONLY | O_TRUNC;
    if (fail_if_exists)
      oflag |= O_EXCL;
    if ((outfile = ::open(to_p.c_str(), oflag, from_stat.st_mode))< 0)
    {
      int open_errno = errno;
      if (infile < 0)
        ::close(infile);
      errno = open_errno;
      return false;
    }

    ssize_t sz, sz_read=1, sz_write;
    while (sz_read > 0
      && (sz_read = ::read(infile, buf.get(), buf_sz))> 0)
    {
      // Allow for partial writes - see Advanced Unix Programming (2nd Ed.),
      // Marc Rochkind, Addison-Wesley, 2004, page 94
      sz_write = 0;
      do
      {
        if ((sz = ::write(outfile, buf.get() + sz_write,
          sz_read - sz_write))< 0)
        { 
          sz_read = sz; // cause read loop termination
          break;        //  and error to be thrown after closes
        }
        sz_write += sz;
      } while (sz_write < sz_read);
    }

    if (::close(infile)< 0)sz_read = -1;
    if (::close(outfile)< 0)sz_read = -1;

    return sz_read >= 0;
  }

}
