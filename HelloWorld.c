#include <stdio.h>

#define SmartLog(format, ...)    printf("SmartLife " format "\r\n", ##__VA_ARGS__)
#define SmartTrace(format, ...)  printf("SmartLife [%s(%d)] " format "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)

void main(void){
     while( true ){
         SmartLog("Hello World");
         SmartTrace("Debug Trace The Code");
     }
}
