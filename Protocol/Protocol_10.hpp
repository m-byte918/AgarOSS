#pragma once
#include "Protocol.hpp"

class Protocol_10 : public Protocol {
public:
    Protocol_10(Player *owner) : Protocol(owner) {}
};