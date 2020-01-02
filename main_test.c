#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define log(format, ...) printf(format "\r\n", ##__VA_ARGS__)

int main(void)
{
    log("hello world");
    return 0;
}
