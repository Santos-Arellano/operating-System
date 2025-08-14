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
  if (fd < 0) {
    perror("Error al abrir el archivo");
    return 1;
  }

  sz = read(fd, c, 10);
  if (sz < 0) {
    perror("Error al leer el archivo");
    close(fd);
    return 1;
  }

  c[sz] = '\0';

  printf("Se llamó a read(%d, c, 10). Devolvió que %d bytes fueron leídos.\n", fd, sz);
  printf("Esos bytes son los siguientes: %s\n", c);

  close(fd);
  free(c);
  return 0;
}
