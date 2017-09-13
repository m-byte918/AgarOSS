#include "./Modules/Utils.h"
#include "./Entity/Entity.h"

class Player {
private:
    Position    _mouse = { 0, 0 };
    std::string _name;
public:
    PlayerCell *_cell;
    Player();
};
