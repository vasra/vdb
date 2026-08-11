#include <cstring>
#include <editline/readline.h>
#include <libvdb/error.hpp>
#include <libvdb/process.hpp>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

void
print_stop_reason(const vdb::Process& process, vdb::StopReason reason)
{
  std::cout << "Process " << process.pid() << ' ';

  switch (reason.reason) {
    case vdb::ProcessState::exited:
      std::cout << "exited with status " << static_cast<int>(reason.info);
      break;
    case vdb::ProcessState::terminated:
      std::cout << "terminated with signal " << sigabbrev_np(reason.info);
      break;
    case vdb::ProcessState::stopped:
      std::cout << "stopped with signal " << sigabbrev_np(reason.info);
      break;
  }
  std::cout << std::endl;
}

std::vector<std::string>
split(std::string_view str, char delimiter)
{
  std::vector<std::string> out{};
  std::stringstream ss{ std::string{ str } };
  std::string item;

  while (std::getline(ss, item, delimiter)) {
    out.push_back(item);
  }

  return out;
}

bool
is_prefix(std::string_view str, std::string_view of)
{
  return of.starts_with(str); // std::equal(str.begin(), str.end(), of.begin());
}

void
handle_command(std::unique_ptr<vdb::Process>& process, std::string_view line)
{
  auto args{ split(line, ' ') };
  auto command{ args[0] };

  if (is_prefix(command, "continue")) {
    process->resume();
    auto reason{ process->wait_on_signal() };
    print_stop_reason(*process, reason);
  } else {
    std::cerr << "Unknown command\n";
  }
}

std::unique_ptr<vdb::Process>
attach(int argc, const char** argv)
{
  // passing PID
  if (argc == 3 && argv[1] == std::string_view("-p")) {
    pid_t pid = std::atoi(argv[2]);
    return vdb::Process::attach(pid);
  } else {
    // passing program name
    const char* program_path{ argv[1] };
    return vdb::Process::launch(program_path);
  }
}

void
main_loop(std::unique_ptr<vdb::Process>& process)
{
  char* line{ nullptr };
  while ((line = readline("vdb> ")) != nullptr) {
    std::string line_str;

    if (line == std::string_view("")) {
      free(line);
      if (history_length > 0) {
        line_str = history_list()[history_length - 1]->line;
      }
    } else {
      line_str = line;
      add_history(line);
      free(line);
    }

    if (!line_str.empty()) {
      try {
        handle_command(process, line_str);
      } catch (const vdb::Error& err) {
        std::cout << err.what() << '\n';
      }
    }
  }
}

} // namespace

int
main(int argc, const char** argv)
{
  if (argc == 1) {
    std::cerr << "No arguments given\n";
    return -1;
  }

  try {
    auto process{ attach(argc, argv) };
    main_loop(process);
  } catch (const vdb::Error& err) {
    std::cout << err.what() << '\n';
  }
}
