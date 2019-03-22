#include "QuadTree.hpp"
#include <algorithm> // QuadTree::remove()
#include <assert.h>  // Rect::Rect()

//** Rect **//
Rect::Rect(const Rect &other) noexcept :
    Rect(other._x, other._y, other._width, other._height) { 
}
Rect::Rect(const std::initializer_list<double> &points) {
    assert(points.size() != 3); // Initializer list may only contain 4 doubles.
    update(*points.begin(), *(points.begin()+1), *(points.begin()+2), *(points.begin()+3));
}
Rect::Rect(double X, double Y, double Width, double Height) noexcept {
    update(X, Y, Width, Height);
}

void Rect::setPosition(double X, double Y) noexcept {
    _x = X;
    _y = Y;
    updateEndpoints();
}
void Rect::setSize(double Width, double Height) noexcept {
    _width = Width;
    _height = Height;
    _halfWidth = Width * 0.5f;
    _halfHeight = Height * 0.5f;
    updateEndpoints();
}
void Rect::update(double X, double Y, double Width, double Height) noexcept {
    _x = X;
    _y = Y;
    setSize(Width, Height);
}

double Rect::x() const noexcept { return _x; }
double Rect::y() const noexcept { return _y; }
double Rect::width() const noexcept { return _width; }
double Rect::height() const noexcept { return _height; }
double Rect::halfWidth() const noexcept { return _halfWidth; }
double Rect::halfHeight() const noexcept { return _halfHeight; }
double Rect::left() const noexcept { return _left; }
double Rect::top() const noexcept { return _top; }
double Rect::right() const noexcept { return _right; }
double Rect::bottom() const noexcept { return _bottom; }

// For map in which X increases from left to right and Y increases from bottom to top
bool Rect::contains(const Rect &other) const noexcept {
    if (other._left   < _left)   return false;
    if (other._top    > _top)    return false;
    if (other._right  > _right)  return false;
    if (other._bottom < _bottom) return false;
    return true; // inside bounds
}
bool Rect::intersects(const Rect &other) const noexcept {
    if (std::abs(_x - other._x) * 2 > _width + other._width) return false;
    if (std::abs(_y - other._y) * 2 > _height + other._height) return false;
    return true; // intersection
}

void Rect::updateEndpoints() noexcept {
    _left   = _x - _halfWidth;
    _top    = _y + _halfHeight;
    _right  = _x + _halfWidth;
    _bottom = _y - _halfHeight;
}

//** Collidable **//
Collidable::Collidable(const Rect &_bounds, std::any _data) :
    bound(_bounds),
    data(_data) {
}

//** QuadTree **//
QuadTree::QuadTree() : 
    QuadTree({}, 0, 0) { 
}
QuadTree::QuadTree(const QuadTree &other) : 
    QuadTree(other.bounds, other.capacity, other.maxLevel) { 
}
QuadTree::QuadTree(const Rect &_bound, unsigned _capacity, unsigned _maxLevel) :
    bounds(_bound),
    capacity(_capacity),
    maxLevel(_maxLevel) {
    objects.reserve(_capacity);
    foundObjects.reserve(_capacity);
}

// Inserts an object into this quadtree
bool QuadTree::insert(Collidable *obj) {
    if (obj->qt != nullptr) return false;

    if (!isLeaf) {
        // insert object into leaf
        if (QuadTree *child = getChild(obj->bound))
            return child->insert(obj);
    }
    objects.push_back(obj);
    obj->qt = this;

    // Subdivide if required
    if (isLeaf && level < maxLevel && objects.size() >= capacity) {
        subdivide();
        update(obj);
    }
    return true;
}

// Removes an object from this quadtree
bool QuadTree::remove(Collidable *obj) noexcept {
    if (obj->qt == nullptr)
        return false; // Cannot exist in vector
    if (obj->qt != this)
        return obj->qt->remove(obj);

    auto index = std::find(objects.begin(), objects.end(), obj);

    if (index == objects.end())
        return false;

    objects.erase(index);
    obj->qt = nullptr;
    discardEmptyBuckets();
    return true;
}

// Removes and re-inserts object into quadtree (for objects that move)
bool QuadTree::update(Collidable *obj) {
    if (!remove(obj)) return false;

    // Not contained in this node -- insert into parent
    if (parent != nullptr && !bounds.contains(obj->bound))
        return parent->insert(obj);
    if (!isLeaf) {
        // Still within current node -- insert into leaf
        if (QuadTree *child = getChild(obj->bound))
            return child->insert(obj);
    }
    return insert(obj);
}

// Searches quadtree for objects within the provided boundary and returns them in vector
const std::vector<Collidable*> &QuadTree::getObjectsInBound(const Rect &bound) {
    foundObjects.clear();
    for (const auto &obj : objects) {
        // Only check for intersection with OTHER boundaries
        if (&obj->bound != &bound && obj->bound.intersects(bound))
            foundObjects.push_back(obj);
    }
    if (!isLeaf) {
        // Get objects from leaves
        if (QuadTree *child = getChild(bound)) {
            child->getObjectsInBound(bound);
            foundObjects.insert(foundObjects.end(), child->foundObjects.begin(), child->foundObjects.end());
        } else for (QuadTree *leaf : children) {
            if (leaf->bounds.intersects(bound)) {
                const std::vector<Collidable*> &leafFoundObjects = leaf->getObjectsInBound(bound);
                foundObjects.insert(foundObjects.end(), leafFoundObjects.begin(), leafFoundObjects.end());
            }
        }
    }
    return foundObjects;
}

// Returns total children count for this quadtree
unsigned QuadTree::totalChildren() const noexcept {
    unsigned total = 0;
    if (isLeaf) return total;
    for (QuadTree *child : children)
        total += child->totalChildren();
    return 4 + total;
}

// Returns total object count for this quadtree
unsigned QuadTree::totalObjects() const noexcept {
    unsigned total = (unsigned)objects.size();
    if (!isLeaf) {
        for (QuadTree *child : children)
            total += child->totalObjects();
    }
    return total;
}

const Rect &QuadTree::getBounds() const noexcept {
    return bounds;
}

// Removes all objects and children from this quadtree
void QuadTree::clear() noexcept {
    if (!objects.empty()) {
        for (Collidable *obj : objects)
            obj->qt = nullptr;
        objects.clear();
    }
    if (!isLeaf) {
        for (QuadTree *child : children) {
            child->clear();
            delete child;
        }
        isLeaf = true;
    }
}

// Subdivides into four quadrants
void QuadTree::subdivide() {
    double hw = bounds.halfWidth();
    double hh = bounds.halfHeight();
    double qw = hw * 0.5f;
    double qh = hh * 0.5f;
    double x = 0, y = 0;
    for (unsigned i = 0; i < 4; ++i) {
        switch (i) {
            case 0: x = bounds.x() + qw; y = bounds.y() - qh; break; // Top right
            case 1: x = bounds.x() - qw; y = bounds.y() - qh; break; // Top left
            case 2: x = bounds.x() - qw; y = bounds.y() + qh; break; // Bottom left
            case 3: x = bounds.x() + qw; y = bounds.y() + qh; break; // Bottom right
        }
        children[i] = new QuadTree({ x, y, hw, hh }, capacity, maxLevel);
        children[i]->level = level + 1;
        children[i]->parent = this;
    }
    isLeaf = false;
}

// Discards buckets if all children are leaves and contain no objects
void QuadTree::discardEmptyBuckets() {
    if (!objects.empty()) return;
    if (!isLeaf) {
        for (QuadTree *child : children)
            if (!child->isLeaf || !child->objects.empty())
                return;
    }
    clear();
    if (parent != nullptr)
        parent->discardEmptyBuckets();
}

// Returns child that contains the provided boundary
QuadTree *QuadTree::getChild(const Rect &bound) const noexcept {
    bool right = bound.left() > bounds.x();
    bool left = !right && bound.right() < bounds.x();

    if (bound.bottom() > bounds.y()) {
        if (left)  return children[2]; // bottom left
        if (right) return children[3]; // bottom right
    } else if (bound.top() < bounds.y()) {
        if (left)  return children[1]; // top left
        if (right) return children[0]; // top right
    }
    return nullptr; // Cannot contain boundary -- too large
}

QuadTree::~QuadTree() {
    clear();
}