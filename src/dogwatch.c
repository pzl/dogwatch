#include <stdio.h>
#include "audio-input.h"

int main(int argc, char **argv) {
    int a;
    a = interfaces(7);
    printf("%d\n",a);
    return 0;
}
