#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

int lockfile(int fd, int type) {
  struct flock fl;
  fl.l_type = type;
  fl.l_start = 0;
  fl.l_whence = SEEK_SET;
  fl.l_len = 0;
  return fcntl(fd, F_SETLK, &fl);
}

int main(int argc, char *argv[]){
  int fd = open(argv[1],O_WRONLY);
  errno = 0;
  printf("%d,%s,%d",lockfile(fd,F_WRLCK), strerror(errno),fd);
  fflush(0);
  while(1){

  }
  return 1;
}
