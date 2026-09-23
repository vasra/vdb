module;

#include <cerrno>

export module vdb.error;

import std;

export namespace vdb {

class Error : public std::runtime_error
{
public:
  [[noreturn]]
  static auto send(const std::string& what) -> void
  {
    throw Error(what);
  }

  [[noreturn]]
  static auto send_errno(const std::string& prefix) -> void
  {
    throw Error(prefix + ": " + std::strerror(errno));
  }

private:
  Error(const std::string& what)
    : std::runtime_error(what)
  {
  }
};

} // vdb
