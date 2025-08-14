#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso de", argv[0], "argumentos");
        return 1;
    }

    int num1 = atoi(argv[1]);
    int num2 = atoi(argv[2]);

    if (num1 < num2) {
        printf("%d es menor que %d\n", num1, num2);
    } else if (num1 > num2) {
        printf("%d es mayor que %d\n", num1, num2);
    } else {
        printf("%d es igual a %d\n", num1, num2);
    }

    return 0;
}
