#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>

int main()

{

  int status, fd1;
  if (fork()) {
    while(1) sleep(10);
  } else {
    exit(4);

  }

}
