#pragma once
#include "Protocol.hpp"
#include "Protocol_11.hpp"

class Protocol_18 : public Protocol_11 {
public:
    Protocol_18(Player *owner) : Protocol_11(owner) {}
};