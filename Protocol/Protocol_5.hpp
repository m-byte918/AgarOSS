#pragma once
#include "Protocol_4.hpp"
#include "../Entities/Ejected.hpp"

class Protocol_5 : public Protocol_4 {
public:
    Protocol_5(Player *owner) : 
        Protocol_4(owner) {
    }
    virtual Buffer &updateNodes(const std::vector<e_ptr> &eatNodes, const std::vector<e_ptr> &updNodes,
        const std::vector<e_ptr> &delNodes, const std::vector<e_ptr> &addNodes) {
        buffer.writeUInt8(0x10);

        // Eat record
        buffer.writeUInt16_LE((unsigned short)eatNodes.size());
        for (e_ptr entity : eatNodes) {
            buffer.writeUInt32_LE(entity->killerId());
            buffer.writeUInt32_LE((unsigned)entity->nodeId());
        }
        // Add record
        for (e_ptr entity : addNodes) {
            buffer.writeUInt32_LE((unsigned)entity->nodeId());
            buffer.writeInt32_LE((int)entity->position().x);
            buffer.writeInt32_LE((int)entity->position().y);
            buffer.writeUInt16_LE((unsigned short)entity->radius());

            buffer.writeUInt8(entity->color().r); // red
            buffer.writeUInt8(entity->color().g); // green
            buffer.writeUInt8(entity->color().b); // blue

            unsigned char flags = 0; // extendedFlag

            if (entity->state & isSpiked)
                flags |= 0x01; // has spikes on outline
            if (entity->type == PlayerCell::TYPE && entity->owner()->skinName() != "")
                flags |= 0x04;
            if (entity->state & isAgitated)
                flags |= 0x10;
            if (entity->type == Ejected::TYPE)
                flags |= 0x20;
            buffer.writeUInt8(flags); // flag

            if (flags & 0x04)
                buffer.writeStrNull_UTF8(entity->owner()->skinName());
            if (entity->type == PlayerCell::TYPE)
                buffer.writeStrNull_UCS2(entity->owner()->cellNameUCS2());
            else
                buffer.writeUInt16_LE(0);
        }
        // Update record
        for (e_ptr entity : updNodes) {
            buffer.writeUInt32_LE((unsigned)entity->nodeId());
            buffer.writeInt32_LE((int)entity->position().x);
            buffer.writeInt32_LE((int)entity->position().y);
            buffer.writeUInt16_LE((unsigned short)entity->radius());

            buffer.writeUInt8(entity->color().r); // red
            buffer.writeUInt8(entity->color().g); // green
            buffer.writeUInt8(entity->color().b); // blue

            unsigned char flags = 0; // extendedFlag

            if (entity->state & isSpiked)
                flags |= 0x01; // has spikes on outline
            if (entity->state & isAgitated)
                flags |= 0x10;
            if (entity->type == Ejected::TYPE)
                flags |= 0x20;

            buffer.writeUInt8(flags); // flag
            buffer.writeUInt16_LE(0); // name
        }
        buffer.writeUInt32_LE(0); // stop update record

        // Remove record
        buffer.writeUInt32_LE((unsigned)delNodes.size());
        for (e_ptr entity : delNodes)
            buffer.writeUInt32_LE((unsigned)entity->nodeId());
        return buffer;
    }
};