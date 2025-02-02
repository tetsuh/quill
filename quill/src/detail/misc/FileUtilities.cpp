#include "quill/detail/misc/FileUtilities.h"
#include "quill/QuillError.h" // for QUILL_THROW, QuillError
#include "quill/QuillError.h"
#include "quill/detail/misc/Common.h" // for QUILL_UNLIKELY
#include "quill/detail/misc/Os.h"     // for fsize, localtime_rs, remove_file
#include <cerrno>                     // for errno
#include <cstring>
#include <ctime> // for time_t
#include <iomanip>
#include <iostream>
#include <sstream> // for operator<<, basic_ostream, bas...

namespace quill
{
namespace detail
{
/***/
void fwrite_fully(void const* ptr, size_t size, size_t count, FILE* stream)
{
  size_t const written = std::fwrite(ptr, size, count, stream);

  if (QUILL_UNLIKELY(written < count))
  {
    std::ostringstream error_msg;
    error_msg << "fwrite failed with error message "
              << "errno: \"" << errno << "\" " << strerror(errno);
    QUILL_THROW(QuillError{error_msg.str()});
  }
}

/***/
FILE* open_file(fs::path const& filename, std::string const& mode)
{
  FILE* fp = ::fopen(filename.string().data(), mode.data());

  if (!fp)
  {
    std::ostringstream error_msg;
    error_msg << "fopen for \"" << filename << "\" mode \"" << mode
              << "\" failed with error message errno: \"" << errno << "\" " << strerror(errno);
    QUILL_THROW(QuillError{error_msg.str()});
  }
  return fp;
}

/***/
size_t file_size(fs::path const& filename) { return static_cast<size_t>(fs::file_size(filename)); }

/***/
bool remove_file(fs::path const& filename) noexcept
{
  std::error_code ec;
  fs::remove(filename, ec);

  if (ec)
  {
    std::cerr << "Failed to remove file \"" << filename << "\" error: " << ec.message() << std::endl;
    return false;
  }

  return true;
}

/***/
bool rename_file(fs::path const& previous_file, fs::path const& new_file) noexcept
{
  std::error_code ec;
  fs::rename(previous_file, new_file, ec);

  if (ec)
  {
    std::cerr << "Failed to rename file from \"" << previous_file << "\" to \"" << new_file
              << "\" error: " << ec.message() << std::endl;
    return false;
  }

  return true;
}

/***/
std::pair<std::string, std::string> extract_stem_and_extension(fs::path const& filename) noexcept
{
  // filename and extension
  return std::make_pair((filename.parent_path() / filename.stem()).string(), filename.extension().string());
}

/***/
fs::path append_date_to_filename(fs::path const& filename, std::chrono::system_clock::time_point timestamp, /* = {} */
                                 bool append_time,      /* = false */
                                 Timezone timezone,     /* = Timezone::LocalTime */
                                 bool zero_out_minutes, /* = false */
                                 bool zero_out_seconds /* = false */) noexcept
{
  // Get the time now as tm from user or default to now
  std::chrono::system_clock::time_point const ts_now =
    (timestamp == std::chrono::system_clock::time_point{}) ? std::chrono::system_clock::now() : timestamp;

  time_t time_now = std::chrono::system_clock::to_time_t(ts_now);
  tm now_tm;

  if (timezone == Timezone::GmtTime)
  {
    detail::gmtime_rs(&time_now, &now_tm);
  }
  else
  {
    detail::localtime_rs(&time_now, &now_tm);
  }

  if (zero_out_minutes)
  {
    now_tm.tm_min = 0;
  }

  if (zero_out_seconds)
  {
    now_tm.tm_sec = 0;
  }

  // Get base file and extension
  std::pair<std::string, std::string> const stem_ext = detail::extract_stem_and_extension(filename);

  // Construct a filename
  std::stringstream ss;
  if (append_time)
  {
    ss << std::setfill('0') << stem_ext.first << "_" << std::setw(4) << now_tm.tm_year + 1900 << "-"
       << std::setw(2) << now_tm.tm_mon + 1 << "-" << std::setw(2) << now_tm.tm_mday << "_"
       << std::setw(2) << now_tm.tm_hour << "-" << std::setw(2) << now_tm.tm_min << "-"
       << std::setw(2) << now_tm.tm_sec << stem_ext.second;
  }
  else
  {
    ss << std::setfill('0') << stem_ext.first << "_" << std::setw(4) << now_tm.tm_year + 1900 << "-"
       << std::setw(2) << now_tm.tm_mon + 1 << "-" << std::setw(2) << now_tm.tm_mday << stem_ext.second;
  }

  return ss.str();
}

/***/
fs::path append_index_to_filename(fs::path const& filename, uint32_t index) noexcept
{
  if (index == 0u)
  {
    return filename;
  }

  // Get base file and extension
  std::pair<std::string, std::string> const stem_ext = detail::extract_stem_and_extension(filename);

  // Construct a filename
  std::stringstream ss;
  ss << stem_ext.first << "." << index << stem_ext.second;
  return ss.str();
}

} // namespace detail
} // namespace quill