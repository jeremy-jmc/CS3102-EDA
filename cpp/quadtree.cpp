// #pragma GCC optimize("Ofast")
// #pragma GCC target("sse4")
#include <iostream>
#include <vector>
#include <queue>
#include <random>
#include <utility>
using namespace std;

typedef pair<int, int> par;

// * UTILITIES
ostream &operator<<(ostream &os, const par &p)
{
    os << "(" << p.first << "," << p.second << ")";
    return os;
}

// * QUADTREE
/*
    Convention:
        - In the children vector:
            children[0] is NW, children[1] is NE, children[2] is SW and children[3] is SE

          N
        W x E
          S
*/
enum CP
{
    NW,
    NE,
    SW,
    SE
}; // 0 1 2 3 4

const int NINF = INT_MIN, INF = INT_MAX;

int correct_quad(const par &center, const par &new_point)
{
    if (center.first > new_point.first &&
        center.second <= new_point.second)
        return NW; // For NW
    else if (center.first <= new_point.first &&
             center.second <= new_point.second)
        return NE;
    else if (center.first > new_point.first &&
             center.second > new_point.second)
        return SW; // For SW
    else
        return SE;
}

struct Node;
class Rectangle
{
public:
    double xmin, xmax, ymin, ymax;

    Rectangle(double xmin, double xmax, double ymin, double ymax)
        : xmin(xmin), xmax(xmax), ymin(ymin), ymax(ymax) {}

    bool contains(const Node &p) const;

    bool intersects(const Rectangle &other) const;
};

struct Node
{
    par point_coords; // point_coords.first == x    &&   point_coords.second == y
    vector<Node *> children;
    bool leaf = false;
    Node()
    {
        this->children.resize(4);
    }
    Node(par point)
    {
        this->point_coords = point;
        children.resize(4);
    }

    void print(ostream &os, int depth, const string &label) const
    {
        for (int i = 1; i < depth; ++i)
            os << "    |";
        if (depth > 0)
            os << "----";

        os << label << "(" << point_coords.first << ", " << point_coords.second << ")" << endl;

        int i = 0;
        for (Node *child : children)
        {
            if (child != nullptr)
            {
                string next_label;
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

    void rangeQuery(const Rectangle& rect, std::vector<Node>& result, const Rectangle& nodeRect) const
    {
        if (rect.contains(*this))
            result.push_back(*this);
        if (this->leaf)
            return;
        
        if (nodeRect.intersects(rect))
        {
            for (int i = 0; i < 4; i++)
            {
                if (children[i] != nullptr)
                {
                    // xmin, xmax, ymin, ymax
                    switch (i)
                    {
                    case NW:
                    {
                        auto r = Rectangle(NINF, point_coords.first, point_coords.second, INF);
                        if (rect.intersects(r))
                            children[i]->rangeQuery(Rectangle(
                                rect.xmin , point_coords.first, point_coords.second, rect.ymax
                            ), result, r);
                        /*
                        xmin, ymax          xmax, ymax
                                     X
                        xmin, ymin          xmax, ymin
                        */
                        break;
                    }
                    case NE:
                    {
                        auto r = Rectangle(point_coords.first, INF, point_coords.second, INF);
                        if (rect.intersects(r))
                            children[i]->rangeQuery(Rectangle(
                                point_coords.first, rect.xmax, point_coords.second, rect.ymax
                            ), result, r);
                        break;
                    }
                    case SW:
                    {
                        auto r = Rectangle(NINF, point_coords.first, NINF, point_coords.second);
                        if (rect.intersects(r))
                            children[i]->rangeQuery(Rectangle(
                                rect.xmin, point_coords.first, rect.ymin, point_coords.second
                            ), result, r);
                        break;
                    }
                    case SE:
                    {
                        auto r = Rectangle(point_coords.first, INF, NINF, point_coords.second);
                        if (rect.intersects(r))
                            children[i]->rangeQuery(Rectangle(
                                point_coords.first, rect.xmax, rect.ymin, point_coords.second
                            ), result, r);
                        break;
                    }
                    }
                }
            }
        }
    }
};


bool Rectangle::contains(const Node &p) const
{
    return (p.point_coords.first >= xmin && 
            p.point_coords.first <= xmax) && 
            (p.point_coords.second >= ymin && 
            p.point_coords.second <= ymax);
}

bool Rectangle::intersects(const Rectangle &other) const
{
    return (xmin <= other.xmax && xmax >= other.xmin) &&
            (ymin <= other.ymax && ymax >= other.ymin);
}


class QuadTree
{
    Node *root;
    friend void print(QuadTree &quadtree);

public:
    QuadTree(Node *new_root)
    {
        this->root = new_root;
    }

    void insert(Node *new_node)
    {
        if (root == nullptr)
        {
            root = new_node;
            root->leaf = true;
        }
        Node *aux = root;

        while (aux != nullptr)
        {
            int quad = correct_quad(aux->point_coords, new_node->point_coords);
            if (aux->children[quad] != nullptr)
                aux = aux->children[quad];
            else
            {
                aux->children[quad] = new_node;
                aux->leaf = false;
                aux->children[quad]->leaf = true;
                break;
            }
        }
    }

    vector<Node> rangeQuery(const Rectangle& range) const
    {
        vector<Node> points;
        root->rangeQuery(range, points, Rectangle(NINF, INF, NINF, INF));
        return points;
    }

    void print() const
    {
        if (root != nullptr)
            root->print(std::cout, 0, "");
    }
};

void print_aux(Node *node, int level = 0)
{
    if (node != nullptr)
    {
        for (size_t i = 0; i < level; i++)
            cout << "\t";

        cout << node->point_coords << endl;
        for (int i = 0; i < node->children.size(); i++)
            print_aux(node->children[i], level + 1);
    }
}

void print(QuadTree &quadtree)
{
    print_aux(quadtree.root);
}

int main()
{
#ifndef TEST
    // freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
#endif

    time_t start, end;
    time(&start);
    int l = 10, r = 40;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(l, r); // dist(gen);

    Node *new_node = new Node;
    new_node->point_coords = {dist(gen), dist(gen)};

    QuadTree quadtree(new_node);
    for (int i = 0; i < 20; i++)
    {
        Node *another_node = new Node;
        another_node->point_coords = {dist(gen), dist(gen)};
        quadtree.insert(another_node);
    }

    cout << "================================================\n";
    print(quadtree);
    cout << "================================================\n";
    quadtree.print();
    cout << "================================================\n";

    Rectangle range(10, 20, 30, 40);        // xmin = 10, xmax = 20, ymin = 30, ymax = 40
    vector<Node> pointsInRange = quadtree.rangeQuery(range);

    cout << "Points in range: \n";
    for (const Node& point : pointsInRange)
        std::cout << "(" << point.point_coords.first << ", " << point.point_coords.second << ")" << std::endl;

    time(&end);
    double time_taken = double(end - start);
    cout << "\nTime: " << fixed << time_taken << " sec " << endl;
    // #ifdef LOCAL_TIME
    //     cout << "Time elapsed: " << 1.0 * clock() / CLOCKS_PER_SEC << "s\n";
    // #endif
    return 0;
}