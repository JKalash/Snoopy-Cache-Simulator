//
//  Constants.h
//  2
//
//  Created by Joseph Kalash on 2/29/16.
//  Copyright © 2016 Joseph Kalash. All rights reserved.
//

#ifndef Constants_h
#define Constants_h

#define MEM_TOTAL_CAPACITY 64*1024
#define N_SET_ASSOCIATIVE 2
#define ADDRESS_SIZE 32
#define MEM_BLOCK_SIZE 64
#define TOTAL_CACHE_BLOCKS (MEM_TOTAL_CAPACITY/MEM_BLOCK_SIZE)
#define K_SETS (TOTAL_CACHE_BLOCKS/N_SET_ASSOCIATIVE)
#define BLOCK_BYTE_OFFSET_SIZE (int)log2(MEM_BLOCK_SIZE)
#define BLOCK_INDEX_SIZE (int)log2(K_SETS)
#define BLOCK_TAG_SIZE ADDRESS_SIZE - BLOCK_INDEX_SIZE - BLOCK_BYTE_OFFSET_SIZE

#include "MemoryBlock.h"

typedef enum : int {
    None,
    ReadMiss,
    WriteMiss,
} MessageType;

//A Message needs to hold information about its type as well as the memory address we're dealing with
struct Message {
    MessageType type;
    MemBlock::MemAddress address;
    Message() {
        type = None;
        address = -1;
    }
};

struct RWInstruction {
    int cpuid;
    bool isRead;
    MemBlock::MemAddress address;
};

#endif /* Constants_h */
