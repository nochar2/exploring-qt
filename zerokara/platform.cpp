#ifdef __unix__
#include <unistd.h>
#include <stdio.h>
#include <sys/inotify.h>
#include <pthread.h>

__attribute__((noreturn))
void *exec_yourself(void *arg) {
  int inotify_fd = *(int *)arg;
  char dontcare[1];
  fprintf(stderr, "\nI: The app will be restarted when the ./zerokara binary changes.\n");
  while(1) { // -- spam while the binary is momentarily gone
    ssize_t read_bytes = read(inotify_fd, dontcare, 1);             (void)(read_bytes);
    int minusone_on_fail = execl("./zerokara", "./zerokara", NULL); (void)(minusone_on_fail);
  }
}
void run_auto_restarter(int argc, char **argv) {
  // -- let's try the hot restart thing.
  (void)argc;
  int inotify_fd = inotify_init1(IN_CLOEXEC);
  inotify_add_watch(inotify_fd, argv[0], IN_ATTRIB);
  pthread_t inotify_reader;
  pthread_create(&inotify_reader, NULL, exec_yourself, &inotify_fd);
}

#else
void run_auto_restarter(int argc, char **argv) {
  fprintf(stderr, "E: auto restart is not available on Windows yet.\n");
}
#endif
