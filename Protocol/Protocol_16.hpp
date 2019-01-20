#pragma once
#include "Protocol_11.hpp"

class Protocol_16 : public Protocol_11 {
public:
    Protocol_16(Player *owner) : Protocol_11(owner) {}
};