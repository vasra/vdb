module;

#include <sys/types.h>

export module vdb.process;

import std;

export namespace vdb {

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
  Process& operator=(const Process&) = delete;
  ~Process();

  static auto launch(std::filesystem::path path, bool debug = true)
    -> std::unique_ptr<Process>;
  static auto attach(pid_t pid) -> std::unique_ptr<Process>;

  auto resume() -> void;
  auto wait_on_signal() -> StopReason;
  auto pid() const -> pid_t { return pid_; }
  auto state() const -> ProcessState { return state_; }

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
