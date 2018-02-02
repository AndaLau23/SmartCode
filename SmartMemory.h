#ifndef __SMART_MEMORY_H__
#define __SMART_MEMORY_H__

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define SMARTMEMORY_HEAP_SIZE              (10 * 1024)
#define SMARTMEMORY_ALIGN_SIZE             (4) // The value must be the power of 2
#define SMARTMEMORY_BLOCK_MIN_SIZE         (16)

#define SMARTMEMORY_ALIGN(size, align)     (size_t)(size + (align - 1)) & (~(align - 1))

#define SMARTMEMORY_LOG(format, ...)       do{ printf("GUI " format "\r\n", ##__VA_ARGS__); } while(0)

#define SMARTMEMORY_RTOS_ENTER_CRITICAL()  // Eneter Critical
#define SMARTMEMORY_RTOS_EXIT_CRITICAL()   // Exit   Critical
 
void   *SmartMemory_Copy(void *Dst, const void *Src, size_t Size);
void   *SmartMemory_Move(void *Dst, const void *Src, size_t Size);
void   *SmartMemory_Set(void *Dst, int Value, size_t Size);
int     SmartMemory_Cmp(const void *Dst, const void *Src, size_t Size);
void   *SmartMemory_Chr(const void *Src, int Value, size_t Size);
size_t  SmartMemory_Strnlen(const char *Src, size_t MaxSize);
size_t  SmartMemory_Strlen(const char *Src);
void   *SmartMemory_Strcpy(char *Dst, const char *Src);
void   *SmartMemory_Strncpy(char *Dst, const char *Src, size_t Size);
char   *SmartMemory_Strchr(const char *Src, char Value);
char   *SmartMemory_Strdup(const char *Src);

void   *SmartMemory_Malloc(size_t Size);
void    SmartMemory_Free(void *pData);
void   *SmartMemory_Realloc(void *pData, size_t Size);
void   *SmartMemory_Calloc(size_t Count, size_t Size);
size_t  SmartMemory_GetAvailableBytes(void);
size_t  SmartMemory_GetUsedPercent(void);

#endif
