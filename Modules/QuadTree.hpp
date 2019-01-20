/***************************************
QuadTree, but for a map with its origin
at the center as opposed to the top left
***************************************/

#pragma once
#include <any>
#include <vector>

class Rect {
public:
    Rect(const Rect&) noexcept;
    Rect(const std::initializer_list<double> &points);
    Rect(double X = 0, double Y = 0, double Width = 0, double Height = 0) noexcept;

    void setPosition(double X, double Y) noexcept;
    void setSize(double Width, double Height) noexcept;
    void update(double X, double Y, double Width, double Height) noexcept;

    double x() const noexcept;
    double y() const noexcept;
    double width() const noexcept;
    double height() const noexcept;
    double halfWidth() const noexcept;
    double halfHeight() const noexcept;
    double left() const noexcept;
    double top() const noexcept;
    double right() const noexcept;
    double bottom() const noexcept;

    bool contains(const Rect &other) const noexcept;
    inline bool intersects(const Rect &other) const noexcept;

private:
    double _x          = 0;
    double _y          = 0;
    double _width      = 0;
    double _height     = 0;
    double _halfWidth  = 0;
    double _halfHeight = 0;
    double _left       = 0;
    double _top        = 0;
    double _right      = 0;
    double _bottom     = 0;

    // Keeps left, top, right, bottom updated for Rect::contains()
    // and QuadTree::getChild() methods. More efficient than 
    // calculating each endpoint per call, especially for quadtrees
    // with a large number of non-moving objects
    void updateEndpoints() noexcept;
};

class QuadTree;
struct Collidable {
    friend class QuadTree;
public:
    Rect bound;
    std::any data;

    Collidable(const Rect &_bounds = {}, std::any _data = {});
private:
    QuadTree *qt = nullptr;
    Collidable(const Collidable&) = delete;
};

class QuadTree {
public:
    QuadTree(const Rect &_bound, unsigned _capacity, unsigned _maxLevel);
    QuadTree(const QuadTree&);
    QuadTree();

    bool insert(Collidable *obj);
    bool remove(Collidable *obj) noexcept;
    bool update(Collidable *obj);
    const std::vector<Collidable*> &getObjectsInBound(const Rect &bound);
    unsigned totalChildren() const noexcept;
    unsigned totalObjects() const noexcept;
    const Rect &getBounds() const noexcept;
    void clear() noexcept;

    ~QuadTree();
private:
    bool	  isLeaf = true;
    unsigned  level = 0;
    unsigned  capacity;
    unsigned  maxLevel;
    Rect      bounds;
    QuadTree* parent = nullptr;
    QuadTree* children[4] = { nullptr, nullptr, nullptr, nullptr };
    std::vector<Collidable*> objects, foundObjects;

    void subdivide();
    void discardEmptyBuckets();
    inline QuadTree *getChild(const Rect &bound) const noexcept;
};