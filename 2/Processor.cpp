//
//  Processor.cpp
//  2
//
//  Created by Joseph Kalash on 2/29/16.
//  Copyright © 2016 Joseph Kalash. All rights reserved.
//

#include "Processor.h"
#include "Constants.h"
#include <math.h>
#include <cstdlib>

using namespace std;

Processor::Processor(int cpuid, std::vector<RWInstruction> instructions, Message& bus) {
    this->state = Idle;
    this->cpuid = cpuid;
    this->instructionsProcessed = 0;
    this->bus = &bus;
    
    for(int i = 0; i < K_SETS; ++i) {
        vector<MemBlock> set(N_SET_ASSOCIATIVE, MemBlock());
        this->cache.push_back(set);
    }
    
    //Extract all instructions that need to be executed by this processor
    for(int i = 0; i < instructions.size(); ++i)
        if(instructions[i].cpuid == this->cpuid)
            this->instructionsToExecute.push_back(instructions[i]);
}

//Returns NULL when it's a MISS.
vector<MemBlock> Processor::blockSetForAddress(MemBlock::MemAddress address) {
    MemBlock::MemAddress copy = address;
    MemBlock::MemAddress index = (copy << (BLOCK_TAG_SIZE)) >> (BLOCK_BYTE_OFFSET_SIZE + BLOCK_TAG_SIZE);
    return this->cache[index];
}

bool Processor::isHit(MemBlock::MemAddress address) {
    
    MemBlock::MemAddress copy = address;
    MemBlock::MemAddress addressTag = copy >> (BLOCK_BYTE_OFFSET_SIZE + BLOCK_INDEX_SIZE);
    vector<MemBlock> set = blockSetForAddress(address);
    
    for(int i = 0; i < set.size(); ++i) {
        if (set[i].getBlockTag() == addressTag)
            return true;
    }
    
    return false;
}

bool Processor::taskComplete() {
    return instructionsProcessed >= instructionsToExecute.size();
}

void Processor::prepareNewRequest() {
    if(this->state == Idle)
        this->state = ReadNewRequest;
}

void Processor::doSomething() {
    
    if(instructionsProcessed == this->instructionsToExecute.size())
        return;
    
    RWInstruction nextInstruction = this->instructionsToExecute[instructionsProcessed];
    
    if(nextInstruction.isRead)
        this->read(nextInstruction.address);
    else this->write(nextInstruction.address, 0x0);
    
    this->instructionsProcessed++;
    
    this->state = Executed;
}

void Processor::snoopBus() {
    
    //Actions follow the FSM according to bus requests
    //There can either be a Read or Write Miss messages.
    
    //Make sure there is a message on the bus
    if (bus->type == None || bus->address == -1)
        return;
    
    //Start by going to the proper cache -> Index
    MemBlock::MemAddress copy = bus->address;
    MemBlock::MemAddress index = (copy << (BLOCK_TAG_SIZE)) >> (BLOCK_BYTE_OFFSET_SIZE + BLOCK_TAG_SIZE);
    
    bool didHit = false;
    
    for(int i = 0; i < cache[index].size(); ++i) {
    
        //Invalid case
        if (!cache[index][i].isHit(bus->address))
            continue;
        
        //Block is a hit
        didHit = true;
        
        string oldState = cache[index][i].stateName();
        
        if(bus->type == ReadMiss) {
            //If Exclusive, WB Block & Abort DRAM (not in this simulation) then go to invalid state
            //Otherwise we don't care
            
            
            if(cache[index][i].getState() == MemBlock::Exclusive)
                cache[index][i].setState(MemBlock::Invalidated);
            
            string newState = cache[index][i].stateName();
            
            #pragma omp critical
            cout << "CPU " << this->cpuid << " received a READ MISS for "<< oldState << " block 0x" << hex << bus->address << ". New State: " <<  newState << endl;
        }
    
        else {
            //Bus contains a write miss
            //If Exclusive, WB Block & Abort DRAM (not in this simulation) then go to shared state
            //If shared, go to invalid
            //Otherwise, we don't care
            
            if(cache[index][i].getState() == MemBlock::Exclusive)
                cache[index][i].setState(MemBlock::Shared);
            else if(cache[index][i].getState() == MemBlock::Shared)
                cache[index][i].setState(MemBlock::Invalidated);
            
            string newState = cache[index][i].stateName();
            
            #pragma omp critical
            cout << "CPU " << this->cpuid << " received a WRITE MISS for " << oldState << " block 0x" << hex << bus->address << ". New State: " << newState << ".\n";
        }
        
        break;
    }
    
    if(!didHit) {
        string type = "READ MISS";
        if(bus->type == WriteMiss)
            type = "WRITE MISS";
        #pragma omp critical
        cout << "CPU " << this->cpuid << " received a " << type << " for Invalid block 0x" << hex << bus->address << ". Do Nothing." << endl;
    }

}

void Processor::read(int address) {
    
    //Start by going to the proper cache -> Index
    //Update my block to hold the same tag as the one I'm trying to write
    MemBlock::MemAddress copy = address;
    MemBlock::MemAddress index = (copy << (BLOCK_TAG_SIZE)) >> (BLOCK_BYTE_OFFSET_SIZE + BLOCK_TAG_SIZE);
    
    bool isHit = false;
    string oldName1 = this->cache[index][0].stateName();
    string oldName2 = this->cache[index][1].stateName();
    
    for(int i = 0; i < this->cache[index].size(); ++i) {
        
        if(!this->cache[index][i].isHit(address)) {
            //Read Miss
            switch (this->cache[index][i].getState()) {
                case MemBlock::Invalidated:
                    bus->type = ReadMiss;
                    bus->address = address;
                    this->cache[index][i].setState(MemBlock::Shared);
                    break;
                case MemBlock::Shared:
                    bus->type = ReadMiss;
                    bus->address = address;
                    break;
                case MemBlock::Exclusive:
                    //**WB Cache block** (NOT NEEDED in this simulation)
                    bus->type = ReadMiss;
                    bus->address = address;
                    this->cache[index][i].setState(MemBlock::Shared);
                    break;
            }
        
            //Update my block to hold the same tag as the one I'm trying to write
            this->cache[index][i].setBlockAddress(address);
            continue;
        }
    
        //Read Hit does not require anything to be done.
        isHit = true;
        #pragma omp critical
        cout << "CPU " << this->cpuid << ": READ HIT on " << this->cache[index][i].stateName() << " block 0x" << hex << address << ".\n";
        break;
    }
    
    if(!isHit) {
        #pragma omp critical
        cout << "CPU " << this->cpuid << ": READ MISS on 0x" << hex << address << ". Old States: [" << oldName1<< "-"<< oldName2<< "]. New States: [" <<  this->cache[index][0].stateName() << "-" << this->cache[index][1].stateName() << "].\n";
    }
}

void Processor::write(int address, char* data) {
    
    MemBlock::MemAddress copy = address;
    MemBlock::MemAddress index = (copy << (BLOCK_TAG_SIZE)) >> (BLOCK_BYTE_OFFSET_SIZE + BLOCK_TAG_SIZE);
    
    bool isHit = false;
    string oldName1 = this->cache[index][0].stateName();
    string oldName2 = this->cache[index][1].stateName();
    
    for(int i = 0; i < this->cache[index].size(); ++i) {
        
        if(!this->cache[index][i].isHit(bus->address)) {
            //Write Miss
            switch (this->cache[index][i].getState()) {
                case MemBlock::Invalidated:
                    bus->type = WriteMiss;
                    bus->address = address;
                    this->cache[index][i].setState(MemBlock::Exclusive);
                    break;
                case MemBlock::Shared:
                    bus->type = WriteMiss;
                    bus->address = address;
                    this->cache[index][i].setState(MemBlock::Exclusive);
                    break;
                case MemBlock::Exclusive:
                    //**WB Cache block** (NOT NEEDED in this simulation)
                    bus->type = WriteMiss;
                    bus->address = address;
                    break;
            }
            
            //Update my block to hold the same tag as the one I'm trying to write
            this->cache[index][i].setBlockAddress(address);
            
            continue;
        }
    
        //Write Hit. If shared, go to exclusive else nothing to do
        isHit = true;
        string oldName = this->cache[index][i].stateName();
        if(this->cache[index][i].getState() == MemBlock::Shared)
            this->cache[index][i].setState(MemBlock::Exclusive);
        #pragma omp critical
        cout << "CPU " << this->cpuid << ": WRITE HIT on " << oldName << " block 0x" << hex << address << ". New State: " <<  this->cache[index][i].stateName() << ".\n";
        break;
    }
    
    if(!isHit) {
        #pragma omp critical
        cout << "CPU " << this->cpuid << ": WRITE MISS on 0x" << hex << address << ". Old States: [" << oldName1 << "-" << oldName2 << "]. New States: [" <<  this->cache[index][0].stateName() << "-" << this->cache[index][1].stateName() << "].\n";
    }
}

