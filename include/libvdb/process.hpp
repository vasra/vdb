#ifndef VDB_PROCESS_HPP
#define VDB_PROCESS_HPP

#include <filesystem>
#include <memory>
#include <sys/types.h>

namespace vdb {

enum class ProcessState
{
  stopped,
  running,
  exited,
  terminated
};

struct StopReason
{
  StopReason(int wait_status);

  ProcessState reason;
  std::uint8_t info;
};

class Process
{
public:
  Process() = delete;
  Process(const Process&) = delete;
  Process& operator=(const Process& other) = delete;
  ~Process();

  static std::unique_ptr<Process> launch(std::filesystem::path path,
                                         bool debug = true);
  static std::unique_ptr<Process> attach(pid_t pid);

  void resume();
  StopReason wait_on_signal();
  pid_t pid() const { return pid_; }
  ProcessState state() const { return state_; }

private:
  Process(pid_t pid, bool terminate_on_end, bool is_attached)
    : pid_(pid)
    , terminate_on_end_(terminate_on_end)
    , is_attached_(is_attached)
  {
  }

  pid_t pid_{ 0 };
  bool is_attached_{ true };
  bool terminate_on_end_{ false };
  ProcessState state_{ ProcessState::stopped };
};

} // vdb
#endif
