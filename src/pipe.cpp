module;

#include <fcntl.h>
#include <unistd.h>

module vdb.pipe;

import vdb.error;

import std;

vdb::Pipe::Pipe(bool close_on_exec)
{
  if (pipe2(fds_, close_on_exec ? O_CLOEXEC : 0) < 0) {
    Error::send_errno("Pipe creation failed");
  }
}

vdb::Pipe::~Pipe()
{
  close_read();
  close_write();
}

auto
vdb::Pipe::release_read() -> int
{
  return std::exchange(fds_[read_fd], -1);
}

auto
vdb::Pipe::release_write() -> int
{
  return std::exchange(fds_[write_fd], -1);
}

auto
vdb::Pipe::close_read() -> void
{
  if (fds_[read_fd] != -1) {
    close(fds_[read_fd]);
    fds_[read_fd] = -1;
  }
}

auto
vdb::Pipe::close_write() -> void
{
  if (fds_[write_fd] != -1) {
    close(fds_[write_fd]);
    fds_[write_fd] = -1;
  }
}

auto
vdb::Pipe::read() -> std::vector<std::byte>
{
  char buf[1024];
  int chars_read;
  if ((chars_read = ::read(fds_[read_fd], buf, sizeof(buf))) < 0) {
    Error::send_errno("Could not read from pipe");
  }

  auto bytes{ reinterpret_cast<std::byte*>(buf) };
  return std::vector<std::byte>(bytes, bytes + chars_read);
}

auto
vdb::Pipe::write(std::byte* from, std::size_t bytes) -> void
{
  if (::write(fds_[write_fd], from, bytes) < 0) {
    Error::send_errno("Could not write to pipe");
  }
}
