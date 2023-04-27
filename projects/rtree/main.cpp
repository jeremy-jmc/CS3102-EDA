#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <limits>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <random>


template <template <class, class> class Container, typename T>
std::ostream &operator<<(std::ostream &os, const Container<T, std::allocator<T>> &container)
{
    os << "[ ";
    for (auto el : container)
        os << el << " ";
    os << "]";
    return os;
}


int SCALAR = 20;
const int M = 3;    // 3
const int m = 2;    // 2
const double EPS = 1e-9;
const int NUM_DIMENSIONS = 2;
const double INF = std::numeric_limits<double>::max(), NINF = std::numeric_limits<double>::lowest();

enum SplitType {
    LINEAR_SPLIT,
    QUADRATIC_SPLIT,
    BROWNIE_SPLIT
};

// * POINT

struct Point 
{
    double coords[NUM_DIMENSIONS];
    Point() {}
    Point(double x, double y) 
    {
        coords[0] = x;
        coords[1] = y;
    }
};

std::ostream& operator<<(std::ostream& os, Point& p) 
{   
    os << "( ";
    for (int i = 0; i < NUM_DIMENSIONS; i++)
        os << p.coords[i] << " ";
    os << ")";
    return os;
}


// * MINIMUM BOUNDING BOX

struct MBB {
    Point lower, upper;
    MBB()
    {
        lower = Point(INF, INF);
        upper = Point(NINF, NINF);
    }

    MBB(const Point& p1, const Point& p2) 
    {
        for (int d = 0; d < NUM_DIMENSIONS; d++) 
        {   // updates the lower and upper bounds
            lower.coords[d] = std::min(p1.coords[d], p2.coords[d]);
            upper.coords[d] = std::max(p1.coords[d], p2.coords[d]);
        }
    }

    double expansion_needed(const Point& p) const 
    {
        double expansion = 0;
        for (int d = 0; d < NUM_DIMENSIONS; d++) 
        {   // sum the expansion needed for each dimension
            expansion += std::max(0.0, p.coords[d] - upper.coords[d]);
            expansion += std::max(0.0, lower.coords[d] - p.coords[d]);
        }
        return expansion;
    }

    double expansion_needed(const MBB& mbb)
    {
        return this->area() - mbb.area();
    }

    void expand(const Point& p) 
    {   // performs like "insert"
        for (int d = 0; d < NUM_DIMENSIONS; d++) 
        {
            lower.coords[d] = std::min(lower.coords[d], p.coords[d]);
            upper.coords[d] = std::max(upper.coords[d], p.coords[d]);
        }
    }

    void expand(const MBB& mbb)
    {
        for (int d = 0; d < NUM_DIMENSIONS; d++) 
        {
            lower.coords[d] = std::min(lower.coords[d], mbb.lower.coords[d]);
            upper.coords[d] = std::max(upper.coords[d], mbb.upper.coords[d]);
        }
    }

    double area() const 
    {   // get the area of N dimensions
        double area = 1;
        for (int d = 0; d < NUM_DIMENSIONS; d++)
            area *= upper.coords[d] - lower.coords[d];
        return area;
    }
};

std::ostream& operator<<(std::ostream& os, MBB& mbb) 
{
    os << "MBB: " << mbb.lower << " " << mbb.upper;
    return os;
}

static double point_distance_squared(const Point& p1, const Point& p2) 
{   // get the cuadratic distance between two points. x^2 + y^2 ... + n^2
    double distance_squared = 0;
    for (int d = 0; d < NUM_DIMENSIONS; d++) 
    {
        double diff = p1.coords[d] - p2.coords[d];
        distance_squared += diff * diff;
    }
    return distance_squared;
}

static double point_distance(const Point& p1, const Point& p2) 
{   // sqrt(x^2 + y^2 ... + n^2)
    return std::sqrt(point_distance_squared(p1, p2));
}

// * RTREE NODE

struct RTreeNode 
{
// methods
    RTreeNode(bool is_leaf = true);
    ~RTreeNode() {}

    // * GETTERS BY REFERENCE
    const std::vector<Point>& points() const { return points_; }
    const std::vector<RTreeNode*>& children() const { return children_; }
    const MBB& mbb() const { return mbb_; }

    
    bool is_leaf() const { return is_leaf_; }

    bool is_full() const 
    { 
        if (this->is_leaf())
            return points_.size() >= M;
        return children_.size() > M;
    }

    void insert(const Point& p);
    
    static SplitType split_type;

// atributes
    bool is_leaf_;
    MBB mbb_;
    RTreeNode* parent_ = nullptr;
    std::vector<Point> points_;
    std::vector<RTreeNode*> children_;

    void split();
    void linear_split();
    void quadratic_split();
    void brownie_split();
    RTreeNode* choose_subtree(const Point& p) const;
    double compute_overlap(const MBB& mbb1, const MBB& mbb2) const;
};

Point centroid(std::vector<Point>& points)
{
    Point centroid(0, 0);
    for (auto p : points)
    {
       for  (int i = 0; i < NUM_DIMENSIONS; i++)
            centroid.coords[i] += p.coords[i]; 
    }
    for  (int i = 0; i < NUM_DIMENSIONS; i++)
        centroid.coords[i] /= points.size();
    return centroid;
}

static double rectangle_distance(RTreeNode* r1, RTreeNode* r2)
{   // centroid criteria
    if (r1->is_leaf_ && r2->is_leaf_)
    {
        Point c1 = centroid(r1->points_);
        Point c2 = centroid(r2->points_);
        
        return point_distance(c1, c2);
    }
    else {
        Point c1(
            (r1->mbb_.lower.coords[0] + r1->mbb_.upper.coords[0]) / 2,
            (r1->mbb_.lower.coords[1] + r1->mbb_.upper.coords[1]) / 2
        );
        Point c2(
            (r2->mbb_.lower.coords[0] + r2->mbb_.upper.coords[0]) / 2,
            (r2->mbb_.lower.coords[1] + r2->mbb_.upper.coords[1]) / 2
        );
        return point_distance(c1, c2);
    }
}

RTreeNode::RTreeNode(bool is_leaf) : is_leaf_(is_leaf) 
{
    // ? why initialize the MBB if it's a leaf?
    if (is_leaf)
        mbb_ = MBB();
    this->parent_ = nullptr;
}

std::ostream& operator<<(std::ostream& os, RTreeNode& node)
{
    os << node.mbb_ << " leaf: " << std::boolalpha << node.is_leaf_;
    os << " points: " << node.points_.size();
    os << " children: " << node.children_.size() << std::endl;
    if (node.is_leaf_)
    {
        os << "\t\t";
        os << node.points_;
        os << std::endl;
    }
    return os;
}

void RTreeNode::insert(const Point& p)
{
    points_.push_back(p);
    mbb_.expand(p);
}

RTreeNode* RTreeNode::choose_subtree(const Point& p) const 
{   // choose the subtree that requires minimum expansion, else the one with minimum area
    RTreeNode* best_child = nullptr;
    double min_expansion = INF;

    // -------
    for (RTreeNode* child : this->children_) 
    {
        double expansion = child->mbb_.expansion_needed(p);
        if (expansion < min_expansion) 
        {
            min_expansion = expansion;
            best_child = child;
        } else if (expansion == min_expansion) 
        {
            double area = child->mbb_.area();
            if (area < best_child->mbb_.area())
                best_child = child;
        }
    }
    // -------

    return best_child;
}

void RTreeNode::split() 
{
    switch (split_type) {
        case LINEAR_SPLIT:
            linear_split();
            break;
        case QUADRATIC_SPLIT:
            quadratic_split();
            break;
        case BROWNIE_SPLIT:
            brownie_split();
            break;
        default:
            throw std::runtime_error("Unknown split type");
    }
}

// * SPLITS

SplitType RTreeNode::split_type;

void RTreeNode::linear_split() 
{
    if (!this->is_leaf_)
        throw std::runtime_error("trying to split a non-leaf node as a leaf node");

    std::random_device rd;
    std::mt19937 gen(rd());
    // * LINEAR SPLIT
    std::cout << "splitting leaf" << std::endl;
    std::uniform_int_distribution<> distrib(0, points_.size() - 1);
    
    // * 1. Choose two seeds
    int idx_f = 0, idx_s;       // distrib(gen)
    Point f = points_[idx_f], s;
    double dis = -1;
    for (int i = 0; i < points_.size(); i++)
        if (idx_f != i && point_distance(points_[i], f) > dis)
        {
            idx_s = i;
            dis = point_distance(points_[i], f);
        }
    s = points_[idx_s]; 
    // std::cout << "\tf: " << f << " s: " << s << std::endl;
    
    RTreeNode* brother = new RTreeNode(true);
    brother->parent_ = this->parent_;
    std::vector<Point> points_aux = this->points_;
    this->points_ = std::vector<Point>();
    this->mbb_ = MBB();

    this->insert(f);
    brother->insert(s);
    
    // std::cout << "\tleaf: " << *this;
    // std::cout << "\tbrother: " << *brother;

    // * 2. Assign the remaining points to the seed that requires less area expansion
    for (int i = 0; i < points_aux.size(); i++)
        if (i != idx_f && i != idx_s)
        {
            if (this->mbb_.expansion_needed(points_aux[i]) < brother->mbb_.expansion_needed(points_aux[i]))
            {
                if (this->points_.size() >= m)
                    brother->insert(points_aux[i]);
                else
                    this->insert(points_aux[i]);
            }
            else
            {
                if (brother->points_.size() >= m)
                    this->insert(points_aux[i]);
                else
                    brother->insert(points_aux[i]);
            }
        }

    // * 7. Update the parent node
    if (this->parent_ == nullptr)
        throw std::runtime_error("The node has no parent");
    
    std::cout << " >>> parent this: " << *this->parent_;
    std::cout << " >>> parent brother: " << *brother->parent_;
    brother->parent_->children_.push_back(brother);
}

RTreeNode* linear_split_internal(RTreeNode* node)
{
    if (node->is_leaf())
        throw std::runtime_error("trying to split a leaf node as a internal node\n");
    
    std::cout << "splitting internal node" << std::endl;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::cout << node->points_ << std::endl;
    std::uniform_int_distribution<> distrib(0, node->children_.size() - 1);
    
    // choose two random children (MBBs), and then the farthest one from them
    int idx_f = 0, idx_s;       // distrib(gen)
    RTreeNode* f = node->children_[idx_f], *s;
    double dis = -1;
    for (int i = 0; i < node->children_.size(); i++)
    {
        // std::cout << *node->children_[i] << std::endl;
        // std::cout << rectangle_distance(node->children_[i], f) << std::endl;
        if (idx_f != i && rectangle_distance(node->children_[i], f) > dis)
        {
            idx_s = i;
            dis = rectangle_distance(node->children_[i], f);
        }
    }
    s = node->children_[idx_s];
    std::cout << "\tf: " << *f << "\ts: " << *s;

    // give the half of children to the brother
    RTreeNode* brother = new RTreeNode(false);
    std::vector<RTreeNode*> children_aux = node->children_;
    node->children_ = std::vector<RTreeNode*>();
    node->mbb_ = MBB();

    node->children_.push_back(f);
    node->mbb_.expand(f->mbb_);

    brother->children_.push_back(s);
    brother->mbb_.expand(s->mbb_);

    // reassign the children
    std::cout << "reassign children\n";
    for (int i = 0; i < children_aux.size(); i++)
        if (i != idx_f && i != idx_s)
        {
            // std::cout << "\t\t" << i << " -> " << *children_aux[i];
            if (node->mbb_.expansion_needed(children_aux[i]->mbb_) < brother->mbb_.expansion_needed(children_aux[i]->mbb_))
            {
                if (node->children_.size() >= m)
                {
                    brother->children_.push_back(children_aux[i]);
                    brother->mbb_.expand(children_aux[i]->mbb_);
                }
                else
                {
                    node->children_.push_back(children_aux[i]);
                    node->mbb_.expand(children_aux[i]->mbb_);
                }   
            }
            else
            {
                if (brother->children_.size() >= m)
                {
                    node->children_.push_back(children_aux[i]);
                    node->mbb_.expand(children_aux[i]->mbb_);
                }
                else
                {
                    brother->children_.push_back(children_aux[i]);
                    brother->mbb_.expand(children_aux[i]->mbb_);
                }   
            }
        }

    std::cout << "\tnode: " << *node;
    std::cout << "\tbrother: " << *brother;
    if (node->parent_ == nullptr)
    {
        node->parent_ = new RTreeNode(false);
        node->parent_->children_.push_back(node);
    }
    brother->parent_ = node->parent_;
    brother->parent_->children_.push_back(brother);

    for (RTreeNode* c : brother->children_)
        c->parent_ = brother;
    
    for (RTreeNode* c : node->parent_->children_)
        node->parent_->mbb_.expand(c->mbb_);
    
    std::cout << " >>> parent this: " << *node->parent_;
    std::cout << " >>> parent brother: " << *brother->parent_;
    return node->parent_;
}

void RTreeNode::quadratic_split()
{
    // -------
    // Tu codigo aqui
    // -------
}

void RTreeNode::brownie_split()
{
    // -------
    // Tu codigo aqui
    // -------
}

// * RTREE

class RTree 
{
public:
    RTree(SplitType split_type = LINEAR_SPLIT);
    RTreeNode* get_root() const { return root_; }
    void draw(const RTreeNode* node, SDL_Renderer* renderer) const;
    void print_ascii() const;
    void insert(Point p);
    void insert_non_full(RTreeNode* node, Point p);
private:
    RTreeNode* root_;
    void print_ascii_node(const RTreeNode* node, int depth = 0) const;
};

RTree::RTree(SplitType split_type) 
{
    RTreeNode::split_type = split_type;
    root_ = new RTreeNode(true);
}

void RTree::insert(Point p) 
{
    RTreeNode* r = root_;
    bool flag = true;
    if (r->is_full())
    {
        std::cout << "\t\troot is full" << std::endl;
        RTreeNode* new_root = new RTreeNode(false);
        new_root->mbb_ = r->mbb_;
        new_root->children_.push_back(r);
        
        r->parent_ = new_root;
        if (new_root->children_[0]->is_leaf())
        {
            new_root->children_[0]->insert(p);
            flag = false;
        }
        new_root->children_[0]->split();
        this->root_ = new_root;
        std::cout << "\t\troot is now " << *this->root_ << std::endl;
    }

    r = this->root_;
    r->mbb_.expand(p);

    if (flag)
    {
        std::cout << "\t\tinserting " << p << std::endl;
        insert_non_full(r, p);
    }
    while (this->root_->parent_)    
        this->root_ = this->root_->parent_;
}

void RTree::insert_non_full(RTreeNode* node, Point p)
{
    // std::cout << "\tchildren size: " << node->children_.size() << std::endl;
    
    if (node->is_leaf())
    {
        node->points_.push_back(p);
        node->mbb_.expand(p);
    }
    else 
    {
        // std::cout << "\t\t\tfind non-full subtree" << std::endl;
        node->mbb_.expand(p);
        RTreeNode* subtree = node->choose_subtree(p);
        subtree->mbb_.expand(p);
        if (subtree->is_full() && subtree->is_leaf())
        {
            std::cout << "case #1\n";
            subtree->points_.push_back(p);           
            subtree->split();
        }
        else if (subtree->is_full())
        {
            std::cout << "case #2\n";
            insert_non_full(subtree, p);
        }
        else
        {
            std::cout << "case #3\n";
            insert_non_full(subtree, p);
        }

        if (node->is_full() && !node->is_leaf())
            node->parent_ = linear_split_internal(node);
    }
    if (node->is_full() && !node->is_leaf())
        node->parent_ = linear_split_internal(node);
}

void RTree::draw(const RTreeNode* node, SDL_Renderer* renderer) const 
{
    if (node->is_leaf())
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    else
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

    SDL_Rect rect;
    rect.x = static_cast<int>(node->mbb().lower.coords[0]);
    rect.y = static_cast<int>(node->mbb().lower.coords[1]);
    rect.w = static_cast<int>(node->mbb().upper.coords[0] - node->mbb().lower.coords[0]);
    rect.h = static_cast<int>(node->mbb().upper.coords[1] - node->mbb().lower.coords[1]);

    SDL_RenderDrawRect(renderer, &rect);

    // Draw the points in the leaf nodes
    if (node->is_leaf()) 
    {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

        for (const Point& p : node->points()) 
        {
            // Draw a small rectangle (2x2 pixels) for each point
            SDL_Rect point_rect;
            point_rect.x = static_cast<int>(p.coords[0]) - 1;
            point_rect.y = static_cast<int>(p.coords[1]) - 1;
            point_rect.w = 2;
            point_rect.h = 2;
            SDL_RenderFillRect(renderer, &point_rect);
        }
    }
    for (const auto& child : node->children())
        draw(child, renderer);
}

void RTree::print_ascii() const 
{
    print_ascii_node(root_);
}

void RTree::print_ascii_node(const RTreeNode* node, int depth) const 
{
    if (node == nullptr)
        return;

    std::string indentation(depth, '\t');

    std::cout << depth << ": "
              << indentation << "MBB: ("
              << node->mbb().lower.coords[0] << ", " << node->mbb().lower.coords[1] << "), ("
              << node->mbb().upper.coords[0] << ", " << node->mbb().upper.coords[1] << ") -> ";

    std::cout << "is leaf: " << std::boolalpha << node->is_leaf_;
    if (node->parent_)
        std::cout << "\t" << node->parent_->mbb_ << std::endl;

    if (node->is_leaf()) {
        std::cout << indentation << "Points:\n";
        for (const Point& p : node->points())
            std::cout << indentation << "  (" << p.coords[0] << ", " << p.coords[1] << ")\n";
    } else {
        std::cout << indentation << "Children: " << node->children().size() << std::endl;
        for (const auto& child : node->children())
            print_ascii_node(child, depth + 1);
    }
}


// ! MAIN MODULE

int main() {
    srand(time(NULL));
    SCALAR = 50;
    RTree rtree(LINEAR_SPLIT);
    
    int window_width = 800*2 / 1.5;
    int window_height = 600*2 / 1.5;

    // Insert points

    int test = 1;

    if (test == 1)
    {
        for (int i = 0; i < 50; ++i)
            rtree.insert(Point(10 + rand() % window_width, 10 + rand() % window_height));
    }
    else if (test == 2) {
        std::vector<Point> pt = {
            Point(3, 4), Point(7, 10), Point(14, 15), 
            Point(9, 11), Point(16, 3), Point(13, 13), 
            Point(4, 5), Point(1, 1), Point(12, 2), 
            Point(5, 6), Point(8, 9), Point(17, 1),  
            Point(2, 2),
            Point(15, 12), Point(16, 14), Point(11, 12),
            Point(11, 14), Point(14, 3), 
            Point(2, 3),  Point(6, 12), Point(8, 7),
            Point(10, 4), Point(11, 11), Point(4, 5),
            Point(5, 2), Point(9, 9), Point(13, 8),
            Point(15, 2), Point(16, 10), Point(12, 14),
            Point(14, 4), Point(17, 16), Point(1, 9),
            Point(7, 15), Point(3, 1), Point(10, 1),
            Point(6, 8), Point(10, 3), Point(2, 7),
            Point(5, 3), Point(9, 6), Point(13, 7),
            Point(11, 10), Point(14, 5), Point(16, 9),
            Point(8, 5), Point(12, 12), Point(17, 14),
            Point(4, 9), Point(15, 3), Point(7, 13),
            Point(10, 15), Point(13, 2), Point(3, 1),
        };
        int i = 1;
        for (auto& p : pt)
        {
            for (auto& c : p.coords)
                c *= SCALAR;
            // std::cout << ">>>> " << i << ". INSERT " << p << std::endl;
            rtree.insert(p);
            std::cout << "  >>> RTREE\n";
            // rtree.print_ascii();
            std::cout << std::endl;
            i++;
        }
            
    } else {
        rtree.insert(Point(  0,   0));
        rtree.insert(Point( 10,  10));
        rtree.insert(Point( 20,  20));
        rtree.insert(Point( 30,  30));
        rtree.insert(Point( 40,  40));
        rtree.insert(Point( 50,  50));
        rtree.insert(Point( 60,  60));
        rtree.insert(Point( 70,  70));
        rtree.insert(Point( 80,  80));
        rtree.insert(Point( 90,  90));
        rtree.insert(Point(100, 100));
    }
    
    printf("Done\n");
    rtree.print_ascii();

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create a window
    SDL_Window* window = SDL_CreateWindow(
        "R-Tree Visualization", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        window_width*1.1, window_height*1.1, 
        SDL_WINDOW_SHOWN
    );

    if (!window) 
    {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create a renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, 
        -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer) 
    {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // R-Tree
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    rtree.draw(rtree.get_root(), renderer);
    SDL_RenderPresent(renderer);

    // Wait 
    bool quit = false;
    SDL_Event event;
    while (!quit) 
    {
        while (SDL_PollEvent(&event)) 
        {
            if (event.type == SDL_QUIT)
                quit = true;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                quit = true;
        }
        SDL_Delay(10); // Add a small delay to reduce CPU usage
    }

    // Clean up and exit
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

