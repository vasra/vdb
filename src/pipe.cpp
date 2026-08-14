#include <fcntl.h>
#include <libvdb/error.hpp>
#include <libvdb/pipe.hpp>
#include <unistd.h>
#include <utility>

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

int
vdb::Pipe::release_read()
{
  return std::exchange(fds_[read_fd], -1);
}

int
vdb::Pipe::release_write()
{
  return std::exchange(fds_[write_fd], -1);
}

void
vdb::Pipe::close_read()
{
  if (fds_[read_fd] != -1) {
    close(fds_[read_fd]);
    fds_[read_fd] = -1;
  }
}

void
vdb::Pipe::close_write()
{
  if (fds_[write_fd] != -1) {
    close(fds_[write_fd]);
    fds_[write_fd] = -1;
  }
}

std::vector<std::byte>
vdb::Pipe::read()
{
  char buf[1024];
  int chars_read;
  if ((chars_read = ::read(fds_[read_fd], buf, sizeof(buf))) < 0) {
    Error::send_errno("Could not read from pipe");
  }

  auto bytes{ reinterpret_cast<std::byte*>(buf) };
  return std::vector<std::byte>(bytes, bytes + chars_read);
}

void
vdb::Pipe::write(std::byte* from, std::size_t bytes)
{
  if (::write(fds_[write_fd], from, bytes) < 0) {
    Error::send_errno("Could not write to pipe");
  }
}
