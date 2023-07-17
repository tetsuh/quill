/**
 * Copyright(c) 2020-present, Odysseas Georgoudis & quill contributors.
 * Distributed under the MIT License (http://opensource.org/licenses/MIT)
 */

#pragma once

#include "quill/Fmt.h"                    // for memory_buffer
#include "quill/detail/misc/Attributes.h" // for QUILL_ATTRIBUTE_COLD, QUIL...
#include "quill/handlers/FileHandler.h"   // for FileHandler
#include <chrono>                         // for hours, minutes, nanoseconds
#include <deque>

namespace quill
{

/**
 * Timed Rotating file handler
 */
class TimeRotatingFileHandler final : public FileHandler
{
public:
  /**
   * Constructor
   * @param base_filename base filename
   * @param mode mode to open_file file
   * @param append_to_filename appends extra info to the file
   * @param when 'M', 'H' or 'daily'
   * @param interval Used when 'M' or 'H' is specified
   * @param max_bytes Max file size in bytes
   * @param backup_count Maximum files to keep
   * @param timezone if true gmt time then UTC times are used instead
   * @param at_time used when 'daily' is specified
   * @param file_event_notifier notifies on file events
   * @param do_fsync also fsync when flushing
   */
  TimeRotatingFileHandler(fs::path const& base_filename, std::string const& mode,
                          FilenameAppend append_to_filename, std::string when, uint32_t interval,
                          size_t max_bytes, uint32_t backup_count, Timezone timezone,
                          std::string const& at_time, FileEventNotifier file_event_notifier, bool do_fsync);

  ~TimeRotatingFileHandler() override = default;

  /**
   * Write a formatted log message to the stream
   * @param formatted_log_message input log message to write
   * @param log_event log_event
   */
  QUILL_ATTRIBUTE_HOT void write(fmt_buffer_t const& formatted_log_message,
                                 quill::TransitEvent const& log_event) override;

private:
  static QUILL_ATTRIBUTE_COLD std::chrono::system_clock::time_point _calculate_initial_rotation_tp(
    std::chrono::system_clock::time_point time_now, std::string const& when, Timezone timezone,
    std::chrono::hours at_time_hours, std::chrono::minutes at_time_minutes) noexcept;

  static QUILL_ATTRIBUTE_COLD std::chrono::system_clock::time_point _calculate_rotation_tp(
    std::chrono::system_clock::time_point time_now, std::string const& when, uint32_t interval) noexcept;

  bool _time_rotation(uint64_t log_msg_ts);
  void _size_rotation(size_t log_msg_size);
  void _rotate_files(bool zero_minutes);

private:
  std::deque<std::pair<uint32_t, fs::path>> _created_files; /**< We store in a queue the filenames we created, in order to remove_file them if we exceed _backup_count limit */
  std::string _when;                                        /**< 'M', 'H' or 'daily' */
  std::chrono::system_clock::time_point _file_creation_time; /**< The time we create the file we are writing */
  std::chrono::system_clock::time_point _next_rotation_time; /**< The next rotation time point */
  size_t _max_bytes;                                         /**< Max file size in bytes */
  size_t _current_size{0};                                   /**< Current file size in bytes */
  uint32_t _interval;       /**< Interval when 'M' or 'H' is used */
  uint32_t _backup_count;   /**< Maximum files to keep after rotation */
  Timezone _using_timezone; /**< The timezone used */
  FilenameAppend _append_to_filename;
};
} // namespace quill
