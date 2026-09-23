#include <catch2/catch_test_macros.hpp>
#include <signal.h>
#include <sys/types.h>

import vdb.error;
import vdb.process;

import std;

using namespace vdb;

namespace {

auto
get_process_status(pid_t pid) -> char
{
  std::ifstream stat("/proc/" + std::to_string(pid) + "/stat");
  std::string data;
  std::getline(stat, data);
  auto index_of_last_parenthesis{ data.rfind(')') };
  auto index_of_status_indicator{ index_of_last_parenthesis + 2 };
  return data[index_of_status_indicator];
}

auto
process_exists(pid_t pid) -> bool
{
  auto ret{ kill(pid, 0) };
  return ret != -1 && errno != ESRCH;
}

} // anonymous namespace

TEST_CASE("Process::launch success", "[Process]")
{
  auto proc{ Process::launch("yes") };
  REQUIRE(process_exists(proc->pid()));
}

TEST_CASE("Process::launch no such program", "[Process]")
{
  REQUIRE_THROWS_AS(Process::launch("you_do_not_have_to_be_good"), Error);
}

TEST_CASE("Process::attach success", "[Process]")
{
  auto target{ Process::launch("targets/run_endlessly", false) };
  auto proc{ Process::attach(target->pid()) };
  REQUIRE(get_process_status(target->pid()) == 't');
}

TEST_CASE("Process::attach invalid PID", "[Process]")
{
  REQUIRE_THROWS_AS(Process::attach(0), Error);
}

TEST_CASE("Process::resume success", "[Process]")
{
  {
    auto proc{ Process::launch("targets/run_endlessly") };
    proc->resume();
    auto status{ get_process_status(proc->pid()) };
    auto success{ status == 'R' || status == 'S' };
    REQUIRE(success);
  }

  {
    auto target{ Process::launch("targets/run_endlessly", false) };
    auto proc{ Process::attach(target->pid()) };
    proc->resume();
    auto status{ get_process_status(proc->pid()) };
    auto success{ status == 'R' || status == 'S' };
    REQUIRE(success);
  }
}

TEST_CASE("Process::already terminated", "[Process]")
{
  auto proc{ Process::launch("targets/end_immediately") };
  proc->resume();
  proc->wait_on_signal();
  REQUIRE_THROWS_AS(proc->resume(), Error);
}
