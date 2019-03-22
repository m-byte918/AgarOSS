#pragma once
#include "Protocol.hpp"

class Protocol_6 : public Protocol {
public:
    Protocol_6(Player *owner) : 
        Protocol(owner) {
    }
    virtual Buffer &updateNodes(const std::vector<e_ptr> &eatNodes, const std::vector<e_ptr> &updNodes,
        const std::vector<e_ptr> &delNodes, const std::vector<e_ptr> &addNodes) {
        Protocol::updateNodes(eatNodes, updNodes, delNodes, addNodes);

        for (const e_ptr &entity : addNodes) {
            buffer.writeUInt32_LE((unsigned)entity->nodeId());
            buffer.writeInt32_LE((int)entity->position().x);
            buffer.writeInt32_LE((int)entity->position().y);
            buffer.writeUInt16_LE((unsigned short)entity->radius());

            unsigned char flags = 0; // extendedFlag

            if (entity->state & isSpiked)
                flags |= 0x01; // has spikes on outline
            if (true)
                flags |= 0x02; // has color
            if (entity->type == CellType::PLAYERCELL && entity->owner()) {
                if (entity->owner()->skinName() != "")
                    flags |= 0x04;
                if (entity->owner()->cellName() != "")
                    flags |= 0x08;
            }
            if (entity->state & isAgitated)
                flags |= 0x10;
            if (entity->type == CellType::EJECTED)
                flags |= 0x20;
            buffer.writeUInt8(flags); // flag

            if (flags & 0x02) {
                buffer.writeUInt8(entity->color().r); // red
                buffer.writeUInt8(entity->color().g); // green
                buffer.writeUInt8(entity->color().b); // blue
            }
            if (flags & 0x04)
                buffer.writeStrNull(entity->owner()->skinName());
            if (flags & 0x08)
                buffer.writeStrNull(entity->owner()->cellName());
        }
        for (const e_ptr &entity : updNodes) {
            buffer.writeUInt32_LE((unsigned)entity->nodeId());
            buffer.writeInt32_LE((int)entity->position().x);
            buffer.writeInt32_LE((int)entity->position().y);
            buffer.writeUInt16_LE((unsigned short)entity->radius());

            unsigned char flags = 0; // extendedFlag

            if (entity->state & isSpiked)
                flags |= 0x01; // virus
            if (entity->type == CellType::PLAYERCELL)
                flags |= 0x02; // has color
            if (entity->state & isAgitated)
                flags |= 0x10;
            if (entity->type == CellType::EJECTED)
                flags |= 0x20;
            buffer.writeUInt8(flags); // flag

            if (flags & 0x02) {
                buffer.writeUInt8(entity->color().r); // red
                buffer.writeUInt8(entity->color().g); // green
                buffer.writeUInt8(entity->color().b); // blue
            }
        }
        buffer.writeUInt32_LE(0); // stop update record

        // Remove record
        buffer.writeUInt16_LE((unsigned short)delNodes.size());
        for (const e_ptr &entity : delNodes)
            buffer.writeUInt32_LE((unsigned int)entity->nodeId());
        return buffer;
    }
};