#include "Packet.hpp"
#include "../Entities/Entity.hpp"

class Player;
class UpdateNodes : public Packet {
public:
    UpdateNodes(Player *_Player,
        const std::vector<e_ptr> &_EatNodes,
        const std::vector<e_ptr> &_UpdNodes,
        const std::vector<e_ptr> &_DelNodes,
        const std::vector<e_ptr> &_AddNodes);

    void writeUpdateRecord_4();
    void writeUpdateRecord_5();
    void writeUpdateRecord_6();
    void writeUpdateRecord_11();

    std::string toString();
private:
    Player *player;
    const std::vector<e_ptr> &eatNodes;
    const std::vector<e_ptr> &updNodes;
    const std::vector<e_ptr> &delNodes;
    const std::vector<e_ptr> &addNodes;
};