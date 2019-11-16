#pragma once
#include "Protocol_10.hpp"
#include "../Entities/Food.hpp"
#include "../Entities/Ejected.hpp"

class Protocol_11 : public Protocol_10 {
public:
    Protocol_11(Player *owner): 
        Protocol_10(owner) {
    }
    virtual Buffer &setBorder() {
        buffer.writeUInt8(0x40);
        buffer.writeDouble_LE(map::bounds().left());
        buffer.writeDouble_LE(map::bounds().bottom());
        buffer.writeDouble_LE(map::bounds().right());
        buffer.writeDouble_LE(map::bounds().top());
        buffer.writeUInt32_LE(cfg::game_mode);
        return buffer.writeStrNull_UTF8(cfg::server_name);
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
            if (entity->type == Food::TYPE)
                flags |= 0x80; // extended flags
            buffer.writeUInt8(flags); // flag

            if (flags & 0x80)
                buffer.writeUInt8(0x01); // flags2
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
            if (entity->type == PlayerCell::TYPE)
                flags |= 0x02; // has color
            if (entity->state & isAgitated)
                flags |= 0x10;
            if (entity->type == Ejected::TYPE)
                flags |= 0x20;
            if (entity->type == Food::TYPE)
                flags |= 0x80; // extended flags
            buffer.writeUInt8(flags); // flag

            if (flags & 0x80)
                buffer.writeUInt8(0x01); // flags2
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