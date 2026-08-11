#include <libvdb/error.hpp>
#include <libvdb/process.hpp>
#include <memory>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

vdb::StopReason::StopReason(int wait_status)
{
  if (WIFEXITED(wait_status)) {
    reason = ProcessState::exited;
    info = WEXITSTATUS(wait_status);
  } else if (WIFSIGNALED(wait_status)) {
    reason = ProcessState::terminated;
    info = WTERMSIG(wait_status);
  } else if (WIFSTOPPED(wait_status)) {
    reason = ProcessState::stopped;
    info = WSTOPSIG(wait_status);
  }
}

vdb::Process::~Process()
{
  if (pid_ != 0) {
    int status;
    if (state_ == ProcessState::running) {
      kill(pid_, SIGSTOP);
      waitpid(pid_, &status, 0);
    }
    ptrace(PTRACE_DETACH, pid_, nullptr, nullptr);
    kill(pid_, SIGCONT);

    if (terminate_on_end_) {
      kill(pid_, SIGKILL);
      waitpid(pid_, &status, 0);
    }
  }
}

std::unique_ptr<vdb::Process>
vdb::Process::launch(std::filesystem::path path)
{
  pid_t pid;
  if ((pid = fork()) < 0) {
    Error::send_errno("fork failed");
  }

  if (pid == 0) {
    if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr)) {
      Error::send_errno("Tracing failed");
    }
    if (execlp(path.c_str(), path.c_str(), nullptr) < 0) {
      Error::send_errno("exec failed");
    }
  }

  std::unique_ptr<Process> proc{ new Process(pid, /*terminate_on_end*/ true) };
  proc->wait_on_signal();

  return proc;
}

std::unique_ptr<vdb::Process>
vdb::Process::attach(pid_t pid)
{
  if (pid == 0) {
    Error::send("Invalid PID");
  }
  if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0) {
    Error::send_errno("Could not attach");
  }

  std::unique_ptr<Process> proc{ new Process(pid, /*terminate_on_end*/ false) };
  proc->wait_on_signal();

  return proc;
}

void
vdb::Process::resume()
{
  if (ptrace(PTRACE_CONT, pid_, nullptr, nullptr) < 0) {
    Error::send_errno("Could not resume");
  }
  state_ = ProcessState::running;
}

vdb::StopReason
vdb::Process::wait_on_signal()
{
  int wait_status;
  int options{ 0 };
  if (waitpid(pid_, &wait_status, options) < 0) {
    Error::send_errno("waitpid failed");
  }
  StopReason reason(wait_status);
  state_ = reason.reason;
  return reason;
}
