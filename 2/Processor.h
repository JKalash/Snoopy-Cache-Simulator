//
//  Processor.h
//  2
//
//  Created by Joseph Kalash on 3/2/16.
//  Copyright © 2016 Joseph Kalash. All rights reserved.
//

#ifndef Processor_h
#define Processor_h

#include <iostream>
#include <vector>
#include "Constants.h"
#include "MemoryBlock.h"

using namespace std;

class Processor {
public:
    typedef enum : int {
        Idle,
        ReadNewRequest,
        Executed
    } ProcState;
private:
    //N-way set associative can be treated as a 2D array (k rows of N columns, k*n = total blocks)
    vector<vector<MemBlock>> cache;
    vector<MemBlock> blockSetForAddress(MemBlock::MemAddress address);
    bool isHit(MemBlock::MemAddress address);
    
    //Pointer to the bus
    Message* bus;
    
    vector<RWInstruction> instructionsToExecute;
public:
    Processor(int cpuid, std::vector<RWInstruction> instructions, Message& bus);
    ProcState state;
    int cpuid;
    int instructionsProcessed;
    void read(int address);
    void write(int address, char* data);
    void snoopBus();
    bool taskComplete();
    void prepareNewRequest();
    void doSomething();
};

#endif /* Processor_h */
