#include "Packet.hpp"
#include "../Entities/Entities.hpp"

class Player;
class UpdateNodes : public Packet {
public:
    UpdateNodes(Player *_Player,
        const std::vector<Entity*> &_EatNodes,
        const std::vector<Entity*> &_UpdNodes,
        const std::vector<Entity*> &_DelNodes,
        const std::vector<Entity*> &_AddNodes);

    void writeUpdateRecord_4();
    void writeUpdateRecord_5();
    void writeUpdateRecord_6();
    void writeUpdateRecord_11();

    std::string toString();
private:
    Player *player;
    const std::vector<Entity*> &eatNodes;
    const std::vector<Entity*> &updNodes;
    const std::vector<Entity*> &delNodes;
    const std::vector<Entity*> &addNodes;
};