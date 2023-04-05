#pragma once
#include "Point.h"
#include "Rectangle.h"

class QuadTreeNode
{
public:
    Point point;
    std::vector<std::unique_ptr<QuadTreeNode>> children;

    QuadTreeNode(Point point)
    {
        this->point = point;
        children.resize(4);
    }

    bool isLeaf()
    {
        for (const auto &child : children)
        {
            if (child != nullptr)
                return false;
        }
        return true;
    }

    bool insert(Point newPoint)
    {
        int quad = correct_quad(this->point, newPoint);

        if (children[quad] == nullptr)
        {
            children[quad] = std::make_unique<QuadTreeNode>(newPoint);
            return true;
        }
        else
            return children[quad]->insert(newPoint);
    }

    QuadTreeNode *search(Point target)
    {
        if (this->point == target)
            return this;

        int quad = correct_quad(this->point, target);
        if (children[quad] == nullptr)
            return nullptr;
        else
            return children[quad]->search(target);
    }

    // bool remove(Point target) {}

    void rangeQuery(const Rectangle &rect, std::vector<Point> &result, const Rectangle &nodeRect, int level = 0) const
    {
        if (!nodeRect.intersects(rect))
            return;
        
        if (rect.contains(point))
            result.push_back(point);

        int i = 0;
        for (const auto &child : children)
        {
            if (child != nullptr)
            {
                Rectangle _rect, _nodeRect;
                switch (i)
                {
                case NW:
                    // xmin, ymin, xmax, ymax
                    _nodeRect = Rectangle(nodeRect.xmin, point.y, point.x, nodeRect.ymax);
                    _rect = Rectangle(
                        rect.xmin, 
                        max(double(point.y), rect.ymin),
                        min(double(point.x), rect.xmax),
                        rect.ymax
                    );
                    break;
                case NE:
                    _nodeRect = Rectangle(point.x, point.y, nodeRect.xmax, nodeRect.ymax);
                    _rect = Rectangle(
                        max(double(point.x), rect.xmin),
                        max(double(point.y), rect.ymin), 
                        rect.xmax, 
                        rect.ymax
                    );
                    break;
                case SW:
                    _nodeRect = Rectangle(nodeRect.xmin, nodeRect.ymin, point.x, point.y);
                    _rect = Rectangle(
                        rect.xmin,
                        rect.ymin, 
                        min(double(point.x), rect.xmax),
                        min(double(point.y), rect.ymax)
                    );
                    break;
                case SE:
                    _nodeRect = Rectangle(point.x, nodeRect.ymin, nodeRect.xmax, point.y);
                    _rect = Rectangle(
                        max(double(point.x), rect.xmin),
                        rect.ymin,
                        rect.xmax,
                        min(double(point.y), rect.ymax)
                    );
                    break;
                }
                child->rangeQuery(_rect, result, _nodeRect, level+1);
            }
            i++;
        }
        /*
        cardinal points
            N
        W       E   
            S
        
        xmin,ymax---xmax,ymax
        |                   |       
        |                   |
        |         X         |
        |                   |
        |                   |
        xmin,ymin---xmax,ymin
        */
    }

    void print(std::ostream &os, int depth = 0, const std::string &label = "") const
    {
        for (int i = 1; i < depth; ++i)
            os << "    |";
        if (depth > 0)
            os << "----";

        os << label << "(" << point.x << ", " << point.y << ")" << std::endl;

        int i = 0;
        for (const auto &child : children)
        {
            if (child != nullptr)
            {
                std::string next_label;
                switch (i)
                {
                case NW:
                    next_label = "NW: ";
                    break;
                case NE:
                    next_label = "NE: ";
                    break;
                case SW:
                    next_label = "SW: ";
                    break;
                case SE:
                    next_label = "SE: ";
                    break;
                }
                child->print(os, depth + 1, next_label);
            }
            i++;
        }
    }
};
