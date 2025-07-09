#define _GNU_SOURCE
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

__attribute__((section(".loader_size"))) //
const volatile uint64_t _loader_size;

int main(int argc, char *argv[], char *envp[]) {
  const int memfd = memfd_create("embedded_app", 0);
  if (memfd == -1) {
    perror("memfd_create");
    return EXIT_FAILURE;
  }

  const int self_exe = open("/proc/self/exe", O_RDONLY);
  if (self_exe == -1) {
    perror("open self exe");
    return EXIT_FAILURE;
  }

  const uint64_t loader_size = _loader_size;
  if (lseek(self_exe, loader_size, SEEK_SET) == -1) {
    perror("lseek self exe");
    return EXIT_FAILURE;
  }

  struct stat st;
  if (fstat(self_exe, &st) < 0) {
    perror("fstat self exe");
    return EXIT_FAILURE;
  }

  const off_t self_size = st.st_size;
  const size_t application_size = self_size - loader_size;

  if (application_size == 0) {
    fputs("No application to be run\n", stderr);
    return EXIT_SUCCESS;
  }

  const ssize_t written = sendfile(memfd, self_exe, NULL, application_size);
  if (written != (ssize_t)application_size) {
    perror("write to memfd");
    return EXIT_FAILURE;
  }

  fexecve(memfd, argv, envp);

  perror("fexecve");
  return EXIT_FAILURE;
}
