#pragma once
#include "Protocol_16.hpp"

class Protocol_17 : public Protocol_16 {
public:
    Protocol_17(Player *owner): 
        Protocol_16(owner) {
    }
};