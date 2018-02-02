#include "SmartMemory.h"

/*
 * @FileName    SmartMemory
 * @Author      AndaLau(BLE)
 * @CreateDate  2018/1/31 22:30:00 Wednesday
 */

#define SMARTMEMORY_ALIGN_MASK       (SMARTMEMORY_ALIGN_SIZE - 1)
#define SMARTMEMORY_STRUCT_SIZE      ((sizeof(SmartMemoryBlock_t) + SMARTMEMORY_ALIGN_MASK) & (~SMARTMEMORY_ALIGN_MASK))
#define SMARTMEMORY_ALLOCATED_MASK   (1 << ((sizeof(uint32_t) * 8) - 1))

#define SMARTMEMORY_MAGIC_VALUE      (0x7EFEFEFF)

#define SMARTMEMORY_HEAP_ADDR        ((uint32_t)SmartMemory_HeapBuffer)
 
#define SmartMemory_Align(size)      SMARTMEMORY_ALIGN(size + SMARTMEMORY_STRUCT_SIZE, SMARTMEMORY_ALIGN_SIZE) 

typedef uint32_t                     MemcpyType;
typedef uint32_t                     MemsetType;
typedef uint32_t                     MemmoveType;
typedef uint32_t                     MemcmpType;

typedef struct xSmartMemoryBlock     SmartMemoryBlock_t;

struct  xSmartMemoryBlock{
        SmartMemoryBlock_t          *pNextFreeBlcok;
        uint32_t                     BlockSize;
};

static  uint8_t                      SmartMemory_HeapBuffer[SMARTMEMORY_HEAP_SIZE];
static  uint32_t                     SmartMemory_FreeBytes        = 0;
static  uint32_t                     SmartMemory_FreeBytesLastMin = 0;
static  SmartMemoryBlock_t           SmartMemoryBlockFree, *SmartMemoryBlockEnd = NULL;

void *SmartMemory_Copy(void *Dst, const void *Src, size_t Size){
      void  *pReturn  = Dst;
      
      if( Size == 0 ) return pReturn;

      uint8_t *pucDst = (uint8_t *)Dst;
      uint8_t *pucSrc = (uint8_t *)Src;

      if( ((uint32_t)Dst % sizeof(MemcpyType)) != ((uint32_t)Src % sizeof(MemcpyType)) ){
           while( Size -- ) *pucDst ++ = *pucSrc ++;
           return pReturn;
      }
 
      while((uint32_t)pucDst % sizeof(MemcpyType)){
          *pucDst ++ = *pucSrc ++;
          if( --Size == 0 ) return pReturn;
      }
    
      MemcpyType *DST    = (MemcpyType *)pucDst;
      MemcpyType *SRC    = (MemcpyType *)pucSrc;
      
      size_t      Length = Size / sizeof(MemcpyType);

      while( Length >= 4 ){  
         *DST ++  = *SRC ++;  *DST ++  = *SRC ++;
         *DST ++  = *SRC ++;  *DST ++  = *SRC ++; 
          Length -= 4;
      }
      while( Length -- ){
         *DST ++  = *SRC ++; 
      } 
      
      pucDst = (uint8_t *)DST;
      pucSrc = (uint8_t *)SRC;
      
      Length = Size % sizeof(MemcpyType);
      
      while( Length -- ){
         *pucDst ++ = *pucSrc ++;
      }
      
      return pReturn;
}

void *SmartMemory_Move(void *Dst, const void *Src, size_t Size){
      void *pReturn = Dst;
      if( Size == 0 ) return pReturn;
 
      if( (uint32_t)Dst <= (uint32_t)Src || (uint8_t *)Dst >= ((uint8_t *)Src + Size) ){
          return SmartMemory_Copy(Dst, Src, Size);
      }else{
          uint8_t *pucDst = (uint8_t *)Dst + Size;
          uint8_t *pucSrc = (uint8_t *)Src + Size;
      
         if( ((uint32_t)Dst % sizeof(MemmoveType)) != ((uint32_t)Src % sizeof(MemmoveType)) ){
             while( Size -- ) *--pucDst = *--pucSrc;
             return pReturn;
         }      
      
          while( (uint32_t)pucDst % sizeof(MemmoveType) ){
              *--pucDst = *--pucSrc;
              if( --Size == 0 ) return pReturn;
          }
         
          MemmoveType *DST    = (MemmoveType *)pucDst;
          MemmoveType *SRC    = (MemmoveType *)pucSrc;
          
          size_t       Length = Size / sizeof(MemmoveType);
         
          while( Length >= 4 ){
              *--DST  = *--SRC;  *--DST  = *--SRC;
              *--DST  = *--SRC;  *--DST  = *--SRC;    
              Length -= 4;
          }  
          while( Length -- ){
              *--DST  = *--SRC;  
          }
         
          pucDst = (uint8_t *)DST;
          pucSrc = (uint8_t *)SRC;
          Length = Size % sizeof(MemmoveType);
         
          while( Length -- ){
             *--pucDst = *--pucSrc;
          }
         
          return pReturn;
     }   
} 

void *SmartMemory_Set(void *Dst, int Value, size_t Size){
      void       *pReturn = Dst;
      
      if( Size == 0 )     return pReturn;
      
      uint8_t *pucDst   = (uint8_t *)Dst;
        
      while( (uint32_t)(pucDst) % sizeof(MemsetType) ){
          *pucDst ++ = Value;
          if( --Size == 0 ) return pReturn;
      }

      MemsetType *DST     = (MemsetType *)pucDst;
      size_t      Length  = Size / sizeof(MemsetType);
      
      if( Length ){
        MemsetType IntValue       = 0;
        uint32_t   MemsetTypeSize = sizeof(MemsetType);

        while( MemsetTypeSize -- ) IntValue = (IntValue << 8) | Value; 
        
        while( Length >= 4 ){
            *DST ++  = IntValue;  *DST ++  = IntValue;
            *DST ++  = IntValue;  *DST ++  = IntValue;      
             Length -= 4;
        }
         while( Length -- ){
            *DST ++   = IntValue;
        }
        pucDst = (uint8_t *)DST;
      }
      
      Length = Size % sizeof(MemsetType);
      while( Length -- ){
          *pucDst ++ = Value;
      }
      
      return pReturn;
}

int   SmartMemory_Cmp(const void *Dst, const void *Src, size_t Size){
      if( Size == 0 ) return 0;

      if( ((uint32_t)Dst % sizeof(MemcmpType)) != ((uint32_t)Src % sizeof(MemcmpType)) ){
          while( Size -- ){
              if( *pucDst ++ !=  *pucSrc ++ ) return (int)(*(pucDst - 1) - *(pucSrc - 1));
          }
          return 0;
      }

      const uint8_t *pucDst = (const uint8_t *)Dst;
      const uint8_t *pucSrc = (const uint8_t *)Src;

      while( (uint32_t)pucDst % sizeof(MemcmpType) ){
         if( *pucDst ++ != *pucSrc ++ ){
              return (int)(*(pucDst - 1) - *(pucSrc - 1));
         }
         if( --Size == 0 ) return 0;
      }

      const MemcmpType *DST = (const MemcmpType *)pucDst;
      const MemcmpType *SRC = (const MemcmpType *)pucSrc;

      size_t Length = Size / sizeof(MemcmpType);	
      
      while( Length -- ){
          if( *DST ++ != *SRC ++ ){
              return (int)(*(DST - 1) - *(SRC - 1));
          }
      }

      pucDst = (const uint8_t *)DST;
      pucSrc = (const uint8_t *)SRC;

      Length = Size % sizeof(MemcmpType);

      while( Length --){
          if( *pucDst ++ !=  *pucSrc ++ ){
              return (int)(*(pucDst - 1) - *(pucSrc - 1));
          }
      }
      
      return 0;
}

void *SmartMemory_Chr(const void *Src, int Value, size_t Size){
      if( Size == 0 ) return NULL;
      
      const uint8_t *pucSrc      = (const uint8_t *)Src;
      uint8_t        SearchValue = (uint8_t)Value;

      while( (uint32_t)pucSrc % sizeof(uint32_t) ){
          if( *pucSrc == SearchValue ) return (void *)pucSrc;
          pucSrc ++;
          if( --Size == 0 ) return NULL;
      }

      uint32_t *SRC           = (uint32_t *)pucSrc;
      
      uint32_t IntValue       = SearchValue | (SearchValue << 8);
      IntValue               |= IntValue << 16; 

      uint32_t  MagicValue    = SMARTMEMORY_MAGIC_VALUE;

      while( Size >= sizeof(uint32_t) ){
          uint32_t XorValue = *SRC ^ IntValue;
          if( ((XorValue + MagicValue) ^ (~XorValue)) & ~MagicValue ) break;
          SRC  ++;
          Size -= sizeof(uint32_t);
      }

      pucSrc = (const uint8_t *)SRC;
      
      while( Size --  ){
         if( *pucSrc == SearchValue ) return (void *)pucSrc;
         pucSrc ++;
      }
      
      return NULL;
}
 
size_t SmartMemory_Strlen(const char *Src){
       const char *pucSrc = Src;
       
       while( (uint32_t)pucSrc % (sizeof(uint32_t)) ){
           if( *pucSrc == '\0' ) return (size_t)(pucSrc - Src);
           pucSrc ++;
       }

       uint32_t   *Value      = (uint32_t *)pucSrc;
       uint32_t    MagicValue = SMARTMEMORY_MAGIC_VALUE; 

       while((((*Value + MagicValue) ^ (~*Value)) & ~MagicValue) == 0){
            Value ++;
       }

       pucSrc = (const char *)Value;
       
       while( *pucSrc != '\0' ) pucSrc ++;

       return (size_t)(pucSrc - Src); 
}

size_t SmartMemory_Strnlen(const char *Src, size_t MaxSize){
       const char *pucSrc = Src;
       const char *pucEnd = Src + MaxSize;
       
       if( MaxSize == 0 ) return 0;
       
       while( (uint32_t)pucSrc % (sizeof(uint32_t)) ){
           if( *pucSrc == '\0' ){
               if( pucSrc > pucEnd ) pucSrc = pucEnd;
               return (size_t)(pucSrc - Src);
           }
           pucSrc ++;
       }
       
       uint32_t  *SRC        = (uint32_t *)pucSrc;
       uint32_t   MagicValue = SMARTMEMORY_MAGIC_VALUE;
       pucSrc                = pucEnd;
       
       while( (uint32_t)SRC < (uint32_t)pucEnd ){
            if( ((*SRC + MagicValue) ^ (~*SRC)) & ~MagicValue ){
                pucSrc = (const char *)SRC;
                while( *pucSrc != '\0' ) pucSrc ++;
                break;
            }
            SRC  ++;
       }
       
       if( pucSrc > pucEnd ) pucSrc = pucEnd;
       
       return (size_t)(pucSrc - Src);
} 
 
void *SmartMemory_Strcpy(char *Dst, const char *Src){
      return SmartMemory_Copy(Dst, Src, SmartMemory_Strlen(Src) + 1);
}

void *SmartMemory_Strncpy(char *Dst, const char *Src, size_t Size){
      size_t Length = SmartMemory_Strnlen(Src, Size);
      if( Length != Size ) SmartMemory_Set(Dst + Length, 0, Size - Length);
      return SmartMemory_Copy(Dst, Src, Length);
}

char *SmartMemory_Strdup(const char *Src){
      size_t Length   = SmartMemory_Strlen(Src) + 1;
      void  *NewAlloc = SmartMemory_Malloc(Length);
      if( NewAlloc == NULL ) return NULL;
      return (char *)SmartMemory_Copy(NewAlloc, Src, Length);
}
 
char *SmartMemory_Strchr(const char *Src, char Value){
      const uint8_t *pucSrc      = (const uint8_t *)Src;
      uint8_t        SearchValue = (uint8_t)Value;

      while( (uint32_t)pucSrc % sizeof(uint32_t) ){
          if( *pucSrc == SearchValue ) return (void *)pucSrc;
          else if( *pucSrc == '\0' ) return NULL;
          pucSrc ++;
      }
      
      uint32_t *SRC       = (uint32_t *)pucSrc;
      
      uint32_t IntValue   = SearchValue | (SearchValue << 8);
      IntValue           |= IntValue << 16; 

      uint32_t MagicValue = SMARTMEMORY_MAGIC_VALUE;

      while(1){
          uint32_t CurrentValue = *SRC ++;
          uint32_t XorValue     = CurrentValue ^ IntValue;
          if( (((CurrentValue + MagicValue) ^ ~CurrentValue) & ~MagicValue) || 
              (((XorValue     + MagicValue) ^ ~XorValue    ) & ~MagicValue) ){
               break;
          }
      }

      pucSrc = (const uint8_t *)(SRC - 1);
      
      while( *pucSrc != '\0' ){
         if( *pucSrc == SearchValue ) return (void *)pucSrc;
         pucSrc ++;
      }
      
      return NULL;
} 
 
#define SmartMemory_IsInitialized() (bool)(SmartMemoryBlockEnd != NULL)

#define SMARTMEMORY_START_ADDR      ((uint32_t)SmartMemory_Align(SMARTMEMORY_HEAP_ADDR))
#define SMARTMEMORY_END_ADDR        ((uint32_t)SmartMemoryBlockEnd)

void  SmartMemory_Init(void){
      uint32_t SmartMemory_HeapStartAddr         = (uint32_t)SmartMemory_HeapBuffer;
      uint32_t SmartMemory_HeapSize              = (uint32_t )SMARTMEMORY_HEAP_SIZE;
    
      uint32_t HeapAlignMaskValue                = SmartMemory_HeapStartAddr & SMARTMEMORY_ALIGN_MASK; 
      if( HeapAlignMaskValue ){
          SmartMemory_HeapStartAddr             += (SMARTMEMORY_ALIGN_SIZE - HeapAlignMaskValue);
          SmartMemory_HeapSize                  -= (SMARTMEMORY_ALIGN_SIZE - HeapAlignMaskValue); 
      } 

      SmartMemoryBlockFree.pNextFreeBlcok        = (SmartMemoryBlock_t *)SmartMemory_HeapStartAddr;
      SmartMemoryBlockFree.BlockSize             = 0;

      uint32_t SmartMemory_HeapEndAddr;
      SmartMemory_HeapEndAddr                    = (uint32_t)SmartMemory_HeapStartAddr + SmartMemory_HeapSize;
      SmartMemory_HeapEndAddr                    = (SmartMemory_HeapEndAddr - SMARTMEMORY_STRUCT_SIZE) & (~SMARTMEMORY_ALIGN_MASK); 

      SmartMemoryBlockEnd                        = (SmartMemoryBlock_t *)SmartMemory_HeapEndAddr;
      SmartMemoryBlockEnd->pNextFreeBlcok        = NULL;
      SmartMemoryBlockEnd->BlockSize             = 0;

      SmartMemoryBlock_t *SmartMemoryBlockFirst;
      SmartMemoryBlockFirst                      = (SmartMemoryBlock_t *)SmartMemory_HeapStartAddr;
      SmartMemoryBlockFirst->BlockSize           = (uint32_t)SmartMemoryBlockEnd - (uint32_t)SmartMemoryBlockFirst;
      SmartMemoryBlockFirst->pNextFreeBlcok      = SmartMemoryBlockEnd;
      
      SmartMemory_FreeBytes                      = SmartMemoryBlockFirst->BlockSize;
      SmartMemory_FreeBytesLastMin               = SmartMemoryBlockFirst->BlockSize;
      
      SMARTMEMORY_LOG("SmartMemoryInitComplete>> HeapStartAddr: %08X, HeapSize: %04X", (uint32_t)SmartMemory_HeapStartAddr, SmartMemory_FreeBytes);
}

void  SmartMemory_InsertBlockToFreeList(SmartMemoryBlock_t *SmartMemoryBlockInsert){
      SmartMemoryBlock_t *pNextBlock = &SmartMemoryBlockFree;
      while( pNextBlock->pNextFreeBlcok < SmartMemoryBlockInsert ){
           pNextBlock = pNextBlock->pNextFreeBlcok;
      }

      SmartMemoryBlock_t *pCurrentBlock;
      pCurrentBlock = pNextBlock;
      if( ((uint32_t)pCurrentBlock + pNextBlock->BlockSize) == (uint32_t)SmartMemoryBlockInsert ){
           pNextBlock->BlockSize  += SmartMemoryBlockInsert->BlockSize;
           SmartMemoryBlockInsert  = pNextBlock;
      }
      
      pCurrentBlock = SmartMemoryBlockInsert;
      if( ((uint32_t)pCurrentBlock + SmartMemoryBlockInsert->BlockSize) == (uint32_t)pNextBlock->pNextFreeBlcok ){
          if( pNextBlock->pNextFreeBlcok != SmartMemoryBlockEnd ){
              SmartMemoryBlockInsert->BlockSize      += pNextBlock->pNextFreeBlcok->BlockSize;
              SmartMemoryBlockInsert->pNextFreeBlcok  = pNextBlock->pNextFreeBlcok->pNextFreeBlcok; 
          }else{
              SmartMemoryBlockInsert->pNextFreeBlcok  = SmartMemoryBlockEnd;
          }
      }else{
          SmartMemoryBlockInsert->pNextFreeBlcok = pNextBlock->pNextFreeBlcok;
      }

      if( pNextBlock != SmartMemoryBlockInsert ){
          pNextBlock->pNextFreeBlcok = SmartMemoryBlockInsert;
      }
}
 
void *SmartMemory_Malloc(size_t Size){
      void *SmartMemory_Return = NULL;
      if( Size == 0 ) return SmartMemory_Return;
      
      SMARTMEMORY_RTOS_ENTER_CRITICAL();
      if( SmartMemory_IsInitialized() == false ) SmartMemory_Init();

      uint32_t ExpectedAllocSize = SmartMemory_Align(Size); 

      if( ExpectedAllocSize <= SmartMemory_FreeBytes ){
          SmartMemoryBlock_t *pPrevBlock = &SmartMemoryBlockFree;
          SmartMemoryBlock_t *pNextBlock = SmartMemoryBlockFree.pNextFreeBlcok;

          while( ( pNextBlock->BlockSize < ExpectedAllocSize ) && ( pNextBlock->pNextFreeBlcok ) ){
               pPrevBlock = pNextBlock;
               pNextBlock = pNextBlock->pNextFreeBlcok;
          }

          if( pNextBlock != SmartMemoryBlockEnd ){
              SmartMemory_Return         = (void *)((uint32_t)pPrevBlock->pNextFreeBlcok + SMARTMEMORY_STRUCT_SIZE);
              pPrevBlock->pNextFreeBlcok = pNextBlock->pNextFreeBlcok;

              if( (int32_t)(pNextBlock->BlockSize - ExpectedAllocSize ) >= SMARTMEMORY_BLOCK_MIN_SIZE ){
                  SmartMemoryBlock_t *SmartMemoryBlockInsert = (SmartMemoryBlock_t *)((uint32_t)pNextBlock + ExpectedAllocSize);
                  SmartMemoryBlockInsert->BlockSize          = pNextBlock->BlockSize - ExpectedAllocSize;
                  pNextBlock->BlockSize                      = ExpectedAllocSize;
                  SmartMemory_InsertBlockToFreeList(SmartMemoryBlockInsert);
              }
          }
          
          SmartMemory_FreeBytes       -= pNextBlock->BlockSize;
          if( SmartMemory_FreeBytes < SmartMemory_FreeBytesLastMin ) SmartMemory_FreeBytesLastMin = SmartMemory_FreeBytes;

          pNextBlock->BlockSize       |= SMARTMEMORY_ALLOCATED_MASK;
          pNextBlock->pNextFreeBlcok   = NULL;
      
      }
      SMARTMEMORY_RTOS_EXIT_CRITICAL();
      
      return SmartMemory_Return;
}

void  SmartMemory_Free(void *pData){
      if( pData == NULL ) return;
      if( ((uint32_t)pData < SMARTMEMORY_START_ADDR) || ((uint32_t)pData >= SMARTMEMORY_END_ADDR) ) return;
      SmartMemoryBlock_t *SmartMemoryBlock = (SmartMemoryBlock_t *)((uint32_t)pData - SMARTMEMORY_STRUCT_SIZE);
      if( (SmartMemoryBlock->BlockSize & SMARTMEMORY_ALLOCATED_MASK) ){
          if( SmartMemoryBlock->pNextFreeBlcok == NULL ){
              SmartMemoryBlock->BlockSize &= (~SMARTMEMORY_ALLOCATED_MASK);
              SMARTMEMORY_RTOS_ENTER_CRITICAL();
              SmartMemory_FreeBytes       += SmartMemoryBlock->BlockSize;
              SmartMemory_InsertBlockToFreeList(SmartMemoryBlock);
              SMARTMEMORY_RTOS_EXIT_CRITICAL();
          }
      }
}
 
size_t SmartMemory_GetAvailableBytes(void){
      return (size_t)SmartMemory_FreeBytes;
}

size_t SmartMemory_GetUsedPercent(void){
      return (size_t)((((uint32_t)SMARTMEMORY_HEAP_SIZE - SmartMemory_FreeBytes) * 100) / SMARTMEMORY_HEAP_SIZE);
}
 
void *SmartMemory_Realloc(void *pData, size_t Size){
      if( Size == 0 ){ SmartMemory_Free(pData); return NULL; }
      
      if( pData == NULL ) return SmartMemory_Malloc(Size);
      
      if( ((uint32_t)pData < SMARTMEMORY_START_ADDR) || ((uint32_t)pData >= SMARTMEMORY_END_ADDR) ) return pData;
      
      SMARTMEMORY_RTOS_ENTER_CRITICAL();
      
      SmartMemoryBlock_t *SmartMemoryBlock = (SmartMemoryBlock_t *)((uint32_t)pData - SMARTMEMORY_STRUCT_SIZE);	

      if( Size == SmartMemoryBlock->BlockSize ){ SMARTMEMORY_RTOS_EXIT_CRITICAL();	return pData;	}
      
      size_t ExpectedAllocSize = SmartMemory_Align(Size); 
      size_t CurrentBlockSize  = SmartMemoryBlock->BlockSize & (~SMARTMEMORY_ALLOCATED_MASK);

      if( (int32_t)(CurrentBlockSize - ExpectedAllocSize) >= SMARTMEMORY_BLOCK_MIN_SIZE ){
          SmartMemoryBlock_t *SmartMemoryBlockInsert = (SmartMemoryBlock_t *)((uint32_t)SmartMemoryBlock + ExpectedAllocSize);
          SmartMemoryBlockInsert->BlockSize          = (CurrentBlockSize - ExpectedAllocSize);
          SmartMemoryBlock->BlockSize                = (ExpectedAllocSize | SMARTMEMORY_ALLOCATED_MASK);
          SmartMemory_FreeBytes                     += SmartMemoryBlockInsert->BlockSize;
          SmartMemory_InsertBlockToFreeList(SmartMemoryBlockInsert);
          SMARTMEMORY_RTOS_EXIT_CRITICAL();
          return pData;
      }
      
      void *SmartMemory_Return = SmartMemory_Malloc(Size);

      if( SmartMemory_Return ){
          SmartMemory_Copy(SmartMemory_Return, pData, (Size < SmartMemoryBlock->BlockSize) ? Size : SmartMemoryBlock->BlockSize);
          SmartMemory_Free(pData);
      }else{
          if( SmartMemoryBlock->BlockSize >= Size ) SmartMemory_Return = pData;
      }
      SMARTMEMORY_RTOS_EXIT_CRITICAL();
      
      return SmartMemory_Return;
}

void *SmartMemory_Calloc(size_t Count, size_t Size){
      uint32_t TotalSize = Count * Size;
      void *SmartMemory_Return = SmartMemory_Malloc(TotalSize);
      if( SmartMemory_Return ){
          SmartMemory_Set(SmartMemory_Return, 0, TotalSize);
      }
      return SmartMemory_Return;
}
 
void  SmartMemory_Demo(void){
      SmartMemory_Set(SmartMemory_HeapBuffer, 0 ,SMARTMEMORY_HEAP_SIZE);
      uint8_t *HeapGroups[20];
      while( true ){   
         uint8_t  HeapIndex = 10;
         
         for(int index = 0; index < HeapIndex; index ++){
            uint32_t AllocSize = rand() % 200;
            HeapGroups[index] = SmartMemory_Malloc(AllocSize);
            SMARTMEMORY_LOG("Malloc HeapIndex: %d, 0x%08X, remind: 0x%04X, AllocSize: %04X", index, (uint32_t)HeapGroups[index], SmartMemory_FreeBytes, AllocSize);
         }

         SMARTMEMORY_LOG("MallocState remind: 0x%04X, used: %d%%", SmartMemory_GetAvailableBytes(), SmartMemory_GetUsedPercent());

         for(int index = 0; index < HeapIndex; index ++){
            SmartMemory_Free(HeapGroups[index]);
            SMARTMEMORY_LOG("Free   HeapIndex: %d, 0x%08X, remind: 0x%04X", index, (uint32_t)HeapGroups[index], SmartMemory_FreeBytes);
         }

         SMARTMEMORY_LOG("FreeState   remind: 0x%04X, used: %d%%", SmartMemory_GetAvailableBytes(), SmartMemory_GetUsedPercent());
         
         for(volatile int i = 0; i < 1000000; i ++) __nop();
      }
}

