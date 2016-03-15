//
//  MessageEntity.hpp
//  2
//
//  Created by Joseph Kalash on 3/2/16.
//  Copyright © 2016 Joseph Kalash. All rights reserved.
//

#ifndef MessageEntity_h
#define MessageEntity_h

#include <stdio.h>

class MessageEntity {
    
public:
    typedef enum : int {
        None,
        ReadMiss,
        WriteMiss,
        Invalidate,
        WriteBack,
        WriteBackAbort
    } Message;
    virtual void receiveMessage(Message message) = 0;
};



#endif /* MessageEntity_hpp */
