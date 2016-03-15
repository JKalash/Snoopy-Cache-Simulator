//
//  DRamController.h
//  2
//
//  Created by Joseph Kalash on 3/2/16.
//  Copyright © 2016 Joseph Kalash. All rights reserved.
//

#ifndef DRamController_h
#define DRamController_h

#include "MemoryBlock.h"
#include "MessageEntity.h"

class MemBlock;

class DRamController : public MessageEntity {
    MemBlock *memory;
    MemBlock* memBlockAtAddress(MemBlock::MemAddress address);
public:
    
    DRamController();
    
    MemBlock receiveReadMiss(MemBlock::MemAddress address);
    void receiveMessage(Message message);
};



#endif /* DRamController_h */
