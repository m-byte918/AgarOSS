#pragma once
#include "Protocol_6.hpp"

class Protocol_7 : public Protocol_6 {
public:
    Protocol_7(Player *owner): 
        Protocol_6(owner) {
    }
};