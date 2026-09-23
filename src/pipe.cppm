export module vdb.pipe;

import std;

export namespace vdb {

class Pipe
{
public:
  explicit Pipe(bool close_on_exec);
  ~Pipe();

  auto get_read() const -> int { return fds_[read_fd]; }
  auto get_write() const -> int { return fds_[write_fd]; }
  auto release_read() -> int;
  auto release_write() -> int;
  auto close_read() -> void;
  auto close_write() -> void;

  auto read() -> std::vector<std::byte>;
  auto write(std::byte* from, std::size_t bytes) -> void;

private:
  static constexpr unsigned read_fd{ 0 };
  static constexpr unsigned write_fd{ 1 };
  int fds_[2];
};

} // vdb
