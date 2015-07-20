#include "myspiht.h"

int main()
{
    float* ptr = NULL;
    int srcX = 1;
    int srcY = 1;
    int srcZ = 1;
    char* ptr2 = NULL;
    short nl = 5;

    printf("encode return: %d\n", spihtencode(ptr, srcX, srcY, srcZ, ptr2, nl) );
    printf("decode return: %d\n", spihtdecode(ptr2, ptr, srcX) );

    return 0;
}
