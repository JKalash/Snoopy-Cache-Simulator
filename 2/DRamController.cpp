//
//  DRamController.cpp
//  2
//
//  Created by Joseph Kalash on 2/29/16.
//  Copyright © 2016 Joseph Kalash. All rights reserved.
//

#include <cstdlib>
#include "DRamController.h"
#include "Constants.h"

MemBlock* DRamController::memBlockAtAddress(MemBlock::MemAddress address) {
    
    MemBlock* memBlockToReturn = NULL;
    
    for(int i = 0; i < DRAM_MEMORY_BLOCK_COUNT; i++) {
        if(memory[i].getBlockAddress() == address)
            memBlockToReturn = &memory[i];
    }
    
    return memBlockToReturn;
}

DRamController::DRamController() {
    memory = new MemBlock[DRAM_MEMORY_BLOCK_COUNT];
    for(int i = 0; i< DRAM_MEMORY_BLOCK_COUNT; ++i)
        memory[i].setBlockAddress(i);
}

MemBlock DRamController::receiveReadMiss(MemBlock::MemAddress address) {
    return *(new MemBlock());
}

void DRamController::receiveMessage(Message message) {
    
}