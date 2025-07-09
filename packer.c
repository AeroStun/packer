
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

static const unsigned char loader[] = {
#embed "loader"
};

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
    return 1;
  }

  const char *file_path = argv[1];
  char tmp_path[FILENAME_MAX];
  snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", file_path);

  const int orig_fd = open(file_path, O_RDONLY);
  if (orig_fd < 0) {
    perror("Failed to open original file");
    return 1;
  }

  const int tmp_fd = open(tmp_path, O_WRONLY | O_CREAT | O_TRUNC, 0755);
  if (tmp_fd < 0) {
    perror("Failed to open temporary file");
    close(orig_fd);
    return 1;
  }

  // Write loader to temporary file
  if (write(tmp_fd, loader, sizeof loader) != sizeof loader) {
    perror("Failed to write loader");
    close(orig_fd);
    close(tmp_fd);
    unlink(tmp_path);
    return EXIT_FAILURE;
  }

  // Get original file size
  struct stat stat_buf;
  if (fstat(orig_fd, &stat_buf) < 0) {
    perror("fstat failed");
    close(orig_fd);
    close(tmp_fd);
    unlink(tmp_path);
    return EXIT_FAILURE;
  }

  // Send file using sendfile
  off_t offset = 0;
  ssize_t sent;
  while (offset < stat_buf.st_size) {
    sent = sendfile(tmp_fd, orig_fd, &offset, stat_buf.st_size - offset);
    if (sent <= 0) {
      perror("sendfile failed");
      close(orig_fd);
      close(tmp_fd);
      unlink(tmp_path);
      return 1;
    }
  }

  close(orig_fd);
  close(tmp_fd);

  // Replace original file with temporary file
  if (unlink(file_path) != 0) {
    perror("Failed to remove original file");
    unlink(tmp_path);
    return EXIT_FAILURE;
  }

  if (rename(tmp_path, file_path) != 0) {
    perror("Failed to rename temporary file");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}