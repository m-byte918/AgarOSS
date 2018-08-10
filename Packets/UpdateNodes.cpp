#include "UpdateNodes.hpp"
#include "../Player.hpp"

UpdateNodes::UpdateNodes(Player *_Player,
    const std::vector<Entity*> &_EatNodes, const std::vector<Entity*> &_UpdNodes,
    const std::vector<Entity*> &_DelNodes, const std::vector<Entity*> &_AddNodes) :
    player(_Player), eatNodes(_EatNodes), updNodes(_UpdNodes),
    delNodes(_DelNodes), addNodes(_AddNodes) {

    buffer.writeUInt8(0x10);

    // Eat record
    buffer.writeUInt16_LE(eatNodes.size());
    for (Entity *entity : eatNodes) {
        buffer.writeUInt32_LE(entity->killerId());
        buffer.writeUInt32_LE(entity->nodeId());
    }

    // Update record
    if (player->protocol < 5) writeUpdateRecord_4();
    else if (player->protocol == 5) writeUpdateRecord_5();
    else if (player->protocol < 11) writeUpdateRecord_6();
    else writeUpdateRecord_11();
    buffer.writeUInt32_LE(0); // stop update record

    // Remove record
    buffer.writeUInt16_LE(delNodes.size());
    for (Entity *entity : delNodes)
        buffer.writeUInt32_LE(entity->nodeId());
}

// TODO: implement for protocols < 11
void UpdateNodes::writeUpdateRecord_4() {
}
void UpdateNodes::writeUpdateRecord_5() {
}
void UpdateNodes::writeUpdateRecord_6() {
}

// TODO: make it so code does not have to be re-used per vector iteration
void UpdateNodes::writeUpdateRecord_11() {
    for (Entity *entity : addNodes) {
        buffer.writeUInt32_LE(entity->nodeId());
        buffer.writeInt32_LE((int)entity->getPosition().x);
        buffer.writeInt32_LE((int)entity->getPosition().y);
        buffer.writeUInt16_LE((unsigned short)entity->getRadius());

        unsigned char flags = 0; // extendedFlag

        if (entity->isSpiked)
            flags |= 0x01; // has spikes on outline
        if (true)
            flags |= 0x02; // has color
        if (entity->type == CellType::PLAYERCELL) {
            if (entity->owner->getSkinName() != "")
                flags |= 0x04;
            if (entity->owner->getCellName() != "")
                flags |= 0x08;
        }
        if (entity->isAgitated)
            flags |= 0x10;
        if (entity->type == CellType::EJECTED)
            flags |= 0x20;
        if (entity->type == CellType::FOOD)
            flags |= 0x80; // extended flags
        buffer.writeUInt8(flags); // flag

        if (flags & 0x80)
            buffer.writeUInt8(0x01); // flags2
        if (flags & 0x02) {
            buffer.writeUInt8(entity->getColor().r); // red
            buffer.writeUInt8(entity->getColor().g); // green
            buffer.writeUInt8(entity->getColor().b); // blue
        }
        if (flags & 0x04)
            buffer.writeStr(entity->owner->getSkinName() + "\0");
        if (flags & 0x08)
            buffer.writeStr(entity->owner->getCellName() + "\0");
    }
    for (Entity *entity : updNodes) {
        buffer.writeUInt32_LE(entity->nodeId());
        buffer.writeInt32_LE((int)entity->getPosition().x);
        buffer.writeInt32_LE((int)entity->getPosition().y);
        buffer.writeUInt16_LE((unsigned short)entity->getRadius());

        unsigned char flags = 0; // extendedFlag

        if (entity->isSpiked)
            flags |= 0x01; // virus
        if (entity->type == CellType::PLAYERCELL)
            flags |= 0x02; // has color
        if (entity->isAgitated)
            flags |= 0x10;
        if (entity->type == CellType::EJECTED)
            flags |= 0x20;
        if (entity->type == CellType::FOOD)
            flags |= 0x80; // extended flags
        buffer.writeUInt8(flags); // flag

        if (flags & 0x80)
            buffer.writeUInt8(0x01); // flags2
        if (flags & 0x02) {
            buffer.writeUInt8(entity->getColor().r); // red
            buffer.writeUInt8(entity->getColor().g); // green
            buffer.writeUInt8(entity->getColor().b); // blue
        }
    }
}

std::string UpdateNodes::toString() {
    Packet::toString();
    ss << "\nEatRecord: {"
       << "\n    radius: " << buffer.readUInt16_LE();

    for (Entity *e : eatNodes) {
        ss << "\n    -------------"
            << "\n    killerId: " << buffer.readUInt32_LE()
            << "\n    nodeId: " << buffer.readUInt32_LE();
    }
    ss << "\n}";

    if (!addNodes.empty()) {
        ss << "\nAddRecord: {";

        for (Entity *e : addNodes) {
            ss << "\n    ---------------------"
                << "\n    nodeId: " << buffer.readUInt32_LE()
                << "\n    positionX: " << buffer.readInt32_LE()
                << "\n    positionY: " << buffer.readInt32_LE()
                << "\n    radius: " << buffer.readUInt16_LE();

            unsigned char flags = buffer.readUInt8();

            ss << "\n    flags: " << (unsigned)flags;

            if (flags & 0x80)
                ss << "\n    flags2: " << (unsigned)buffer.readUInt8();
            if (flags & 0x02) {
                ss << "\n    colorR: " << (unsigned)buffer.readUInt8()
                    << "\n    colorG: " << (unsigned)buffer.readUInt8()
                    << "\n    colorB: " << (unsigned)buffer.readUInt8();
            }
            if (flags & 0x04)
                ss << "\n    skinName: " << buffer.readStr(player->getSkinName().size());
            if (flags & 0x08)
                ss << "\n    cellName: " << buffer.readStr(player->getCellName().size());
        }
        ss << "\n}";
    }
    if (!updNodes.empty()) {
        ss << "\nUpdateRecord: {";

        for (Entity *e : updNodes) {
            ss << "\n    ---------------------"
                << "\n    nodeId: " << buffer.readUInt32_LE()
                << "\n    positionX: " << buffer.readInt32_LE()
                << "\n    positionY: " << buffer.readInt32_LE()
                << "\n    radius: " << buffer.readUInt16_LE();

            unsigned char flags = buffer.readUInt8();

            ss << "\n    flags: " << (unsigned)flags;

            if (flags & 0x80)
                ss << "\n    flags2: " << (unsigned)buffer.readUInt8();
            if (flags & 0x02) {
                ss << "\n    colorR: " << (unsigned)buffer.readUInt8()
                    << "\n    colorG: " << (unsigned)buffer.readUInt8()
                    << "\n    colorB: " << (unsigned)buffer.readUInt8();
            }
        }
        ss << "\n}";
    }
    ss << "\nUpdateRecordTerminator: " << buffer.readUInt32_LE()
        << "\nRemoveRecord: {"
        << "\n    radius: " << buffer.readUInt16_LE();

    for (Entity *e : delNodes)
        ss << "    nodeId: " << buffer.readUInt32_LE();

    ss << "\n}";

    return ss.str();
}