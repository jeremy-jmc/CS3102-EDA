#pragma once
#include "QuadTreeNode.h"

class PointQuadTree
{
public:
    std::unique_ptr<QuadTreeNode> root = nullptr;
    PointQuadTree(){};

    bool insert(Point point)
    {
        if (root == nullptr)
        {
            root = std::make_unique<QuadTreeNode>(point);
            return true;
        }
        return root->insert(point);
    }

    QuadTreeNode *search(Point point)
    {
        if (root != nullptr)
            return root->search(point);
        return nullptr;
    }

    // bool remove(Point point){ }

    std::vector<Point> rangeQuery(const Rectangle &range) const
    {
        std::vector<Point> result;
        if (root != nullptr)
            root->rangeQuery(range, result, Rectangle(NINF, NINF, INF, INF));
        return result;
    }

    void print() const
    {
        if (root != nullptr)
            root->print(std::cout, 0, "");
    }
};
