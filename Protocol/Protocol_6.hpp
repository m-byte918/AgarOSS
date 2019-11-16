#pragma once
#include "Protocol.hpp"
#include "../Entities/Ejected.hpp"

class Protocol_6 : public Protocol {
public:
    Protocol_6(Player *owner) : 
        Protocol(owner) {
    }
    virtual Buffer &updateLeaderboardList() {
        buffer.writeUInt8(0x31);
        unsigned len = (unsigned)map::game->leaders.size();
        buffer.writeUInt32_LE(len);
        for (unsigned i = 0; i < len; ++i) {
            Player *p = map::game->leaders[i];
            if (!p || p->state() != PlayerState::PLAYING)
                continue;
            buffer.writeUInt32_LE(p == player);
            buffer.writeStrNull_UTF8(p->cellNameUTF8());
        }
        return buffer;
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

            unsigned char flags = 0; // extendedFlag

            if (entity->state & isSpiked)
                flags |= 0x01; // has spikes on outline
            if (true)
                flags |= 0x02; // has color
            if (entity->type == PlayerCell::TYPE) {
                if (entity->owner()->skinName() != "") flags |= 0x04;
                if (entity->owner()->cellNameUTF8() != "") flags |= 0x08;
            }
            if (entity->state & isAgitated)
                flags |= 0x10;
            if (entity->type == Ejected::TYPE)
                flags |= 0x20;
            buffer.writeUInt8(flags); // flag

            if (flags & 0x02) {
                buffer.writeUInt8(entity->color().r); // red
                buffer.writeUInt8(entity->color().g); // green
                buffer.writeUInt8(entity->color().b); // blue
            }
            if (flags & 0x04) buffer.writeStrNull_UTF8(entity->owner()->skinName());
            if (flags & 0x08) buffer.writeStrNull_UTF8(entity->owner()->cellNameUTF8());
        }
        // Update record
        for (e_ptr entity : updNodes) {
            buffer.writeUInt32_LE((unsigned)entity->nodeId());
            buffer.writeInt32_LE((int)entity->position().x);
            buffer.writeInt32_LE((int)entity->position().y);
            buffer.writeUInt16_LE((unsigned short)entity->radius());

            unsigned char flags = 0; // extendedFlag

            if (entity->state & isSpiked)
                flags |= 0x01; // virus
            if (true)
                flags |= 0x02; // has color
            if (entity->state & isAgitated)
                flags |= 0x10;
            if (entity->type == Ejected::TYPE)
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
        for (e_ptr entity : delNodes)
            buffer.writeUInt32_LE((unsigned int)entity->nodeId());
        return buffer;
    }
};