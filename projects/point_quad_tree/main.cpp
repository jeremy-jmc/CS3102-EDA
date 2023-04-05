#include "PointQuadTree.h"

int main() {
    int l = 0, r = 50;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(l, r); // dist(gen);


    PointQuadTree quadtree = PointQuadTree();
    int n = 200;

    // Insert points into the quadtree
    quadtree.insert(Point(dist(gen), dist(gen)));

    for (int i = 0; i < n; i++) {
        quadtree.insert(Point(dist(gen), dist(gen)));      // rand() % n
    }

    quadtree.print();

    // Search 
    Point searchPoint(dist(gen), dist(gen));
    QuadTreeNode* foundNode = quadtree.search(searchPoint);

    if (foundNode != nullptr) {
        std::cout << "Point (" << foundNode->point.x << ", " << foundNode->point.y << ") found in the quadtree." << std::endl;
    } else {
        std::cout << "Point (" << searchPoint.x << ", " << searchPoint.y << ") not found in the quadtree." << std::endl;
    }

    // Range query
    Rectangle range(10, 20, 30, 40); // xmin = 10, ymin = 20, xmax = 30, ymax = 40
    std::vector<Point> pointsInRange = quadtree.rangeQuery(range);

    cout << "Points in range\tX: " << 
        range.xmin << " - " << range.xmax << " Y: " << range.ymin << " - " << range.ymax << endl;
    
    for (const Point& point : pointsInRange) {
        std::cout << "(" << point.x << ", " << point.y << ")" << std::endl;
    }

    return 0;
}

/*
cardinal points
    N
W       E   
    S
*/