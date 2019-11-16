#pragma once
#include "Protocol_9.hpp"

class Protocol_10 : public Protocol_9 {
public:
    Protocol_10(Player *owner): 
        Protocol_9(owner) {
    }
};