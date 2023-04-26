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


const int M = 3;
const int m = 2;
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
        int scalar = 20;
        coords[0] = x * scalar;
        coords[1] = y * scalar;
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

    void expand(const Point& p) 
    {   // performs like "insert"
        for (int d = 0; d < NUM_DIMENSIONS; d++) 
        {
            lower.coords[d] = std::min(lower.coords[d], p.coords[d]);
            upper.coords[d] = std::max(upper.coords[d], p.coords[d]);
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

    bool overflow() const 
    { 
        if (this->is_leaf())
            return points_.size() > M;
        return children_.size() > M;
    }

    RTreeNode* insert(const Point& p);
    
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

RTreeNode::RTreeNode(bool is_leaf) : is_leaf_(is_leaf) 
{
    // ? why initialize the MBB if it's a leaf?
    if (is_leaf)
        mbb_ = MBB();
}

std::ostream& operator<<(std::ostream& os, RTreeNode& node)
{
    os << node.mbb_ << " leaf: " << std::boolalpha << node.is_leaf_ << " points: " << node.points_.size() << " children: " << node.children_.size() << std::endl;
    return os;
}

RTreeNode* RTreeNode::insert(const Point& p) 
{
    if (is_leaf_) 
    {
        points_.push_back(p);
        mbb_.expand(p);

        // If the current node is a leaf and has no parent, create a new parent and expand the MBB
        if (parent_ == nullptr && points_.size() > M)
        {   // If the node is root
            // std::cout << "splitting root" << std::endl;
            parent_ = new RTreeNode(RTreeNode(false));
            for (const Point& point : points_)
                parent_->mbb_.expand(point);
            
            // std::cout << " -> parent: " << *this->parent_;

            split();
            
            // for (auto child : parent_->children_)
            //     std::cout << "  -> child: " << *child;
            
            parent_->children_.push_back(new RTreeNode(*this));
            return parent_;
        }
        else if (points_.size() > M)
        {
            // std::cout << "splitting leaf" << std::endl;
            split();
            // std::cout << " -> parent: " << *this->parent_;
            // for (auto child : parent_->children_)
            //     std::cout << "  -> child: " << *child;
        }
    } else
    {
        // If the current node is not a leaf, choose the best subtree to insert the point
        RTreeNode* subtree = choose_subtree(p);
        subtree->insert(p);

        // After insert the point, update the MBB
        mbb_.expand(p);
    }
    return this;
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
    // -------
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, points_.size() - 1);

    // choose one random point, and then the farthest point from it
    int idx_f = distrib(gen), idx_s;
    Point f = points_[idx_f], s;

    double dis = -1;
    for (int i = 0; i < points_.size(); i++)
        if (idx_f != i && point_distance(points_[i], f) > dis)
        {
            idx_s = i;
            dis = point_distance(points_[i], f);
        }
    s = points_[idx_s];
    std::cout << "\tf: " << f << " s: " << s << std::endl;
    
    // give the half of points to the brother
    std::vector<Point> points_aux = this->points_;

    RTreeNode* brother = new RTreeNode(true);
    this->points_ = std::vector<Point>();
    this->mbb_ = MBB();

    this->insert(f);
    brother->insert(s);

    // reassign points
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

    // add brother to parent
    if (this->parent_ == nullptr)
        std::runtime_error("The node has no parent");
    
    this->parent_->children_.push_back(brother);
    brother->parent_ = this->parent_;
    
    // std::cout << "\tthis: " << *this;
    // std::cout << "\t\t" << this->points_ << " \t" << this->mbb_ << std::endl;
    // std::cout << "\tbrother: " << *brother;
    // std::cout << "\t\t" << brother->points_ << " \t" << brother->mbb_ << std::endl;
    // -------
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
    std::cout << "\t\tinserting " << p << std::endl;
    root_ = root_->insert(p);
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

    std::string indentation(depth * 2, ' ');

    std::cout << indentation << "MBB: ("
              << node->mbb().lower.coords[0] << ", " << node->mbb().lower.coords[1] << "), ("
              << node->mbb().upper.coords[0] << ", " << node->mbb().upper.coords[1] << ")\n";

    if (node->is_leaf()) {
        std::cout << indentation << "Points:\n";
        for (const Point& p : node->points())
            std::cout << indentation << "  (" << p.coords[0] << ", " << p.coords[1] << ")\n";
    } else {
        std::cout << indentation << "Children:\n";
        for (const auto& child : node->children())
            print_ascii_node(child, depth + 1);
    }
}


// ! MAIN MODULE

int main() {
    srand(time(NULL));
    RTree rtree(LINEAR_SPLIT);
    
    int window_width = 800 / 1.5;
    int window_height = 600 / 1.5;

    // Insert points
    // for (int i = 0; i < 20; ++i)
    //     rtree.insert(Point(rand() % window_width, rand() % window_height));
    
    rtree.insert(Point(7, 10));
    rtree.insert(Point(9, 11));
    rtree.insert(Point(3, 4));
    rtree.insert(Point(4, 5));
    rtree.insert(Point(1, 1));
    rtree.insert(Point(12, 2));
    rtree.insert(Point(14, 15));
    rtree.insert(Point(16, 3));
    rtree.insert(Point(13, 13));
    rtree.insert(Point(5, 6));
    rtree.insert(Point(8, 9));
    rtree.insert(Point(15, 12));
    
    
    // rtree.insert(Point(  0,   0));
    // rtree.insert(Point( 10,  10));
    // rtree.insert(Point( 20,  20));
    // rtree.insert(Point( 30,  30));
    // rtree.insert(Point( 40,  40));
    // rtree.insert(Point( 50,  50));
    // rtree.insert(Point( 60,  60));
    // rtree.insert(Point( 70,  70));
    // rtree.insert(Point( 80,  80));
    // rtree.insert(Point( 90,  90));
    // rtree.insert(Point(100, 100));
    
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
        window_width, window_height, 
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

