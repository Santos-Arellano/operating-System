#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  if (argc != 2) {
    printf("Uso: %s <nombre_archivo>\n", argv[0]);
    return 1;
  }

  int fd, sz;
  char *c = (char *) calloc(100, sizeof(char));

  fd = open(argv[1], O_RDONLY);

  sz = read(fd, c, 10);

  printf("se llamo a read(%d, c, 10). Devolvio que %d bytes fueron leidos.\n", fd, sz);
  c[sz] = '\0';
  printf("Esos bytes son los siguientes: %s\n", c);

  close(fd);
  free(c);
  return 0;
}
