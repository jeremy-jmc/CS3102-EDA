#include "PointQuadTree.h"

int l = 0, r = 50;
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<> dist(l, r); // dist(gen);

void test()
{
    PointQuadTree quadtree = PointQuadTree();
    int n = 50;

    // Insert points into the quadtree
    quadtree.insert(Point(20, 30));   // dist(gen), dist(gen)

    for (int i = 0; i < n; i++) {
        quadtree.insert(Point(rand() % n, rand() % n));      // rand() % n    dist(gen), dist(gen)
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
}


void bulk_loading(PointQuadTree& quadtree, vector<Point>::iterator beg, vector<Point>::iterator en)
{
    if (beg == en)
        return;

    vector<Point>::iterator mid = beg + (en - beg) / 2;
    quadtree.insert(*mid);
    bulk_loading(quadtree, beg, mid);
    bulk_loading(quadtree, mid + 1, en);
}

int main() {
    srand(time(NULL));

    // test();

    vector<Point> points;
    for (int i = 0; i < 50; i++)
        points.push_back(Point(dist(gen), dist(gen)));  // rand() % 50, rand() % 50
    
    sort(points.begin(), points.end(), [](const Point& a, const Point& b) {
        if (a.x == b.x)
            return a.y < b.y;
        return a.x < b.x;
    });

    // cout<<"===================================================="<<endl;
    // for (const Point& point : points)
    //     cout << point.x << " " << point.y << endl;
    // cout<<"===================================================="<<endl;

    PointQuadTree quadtree = PointQuadTree();
    bulk_loading(quadtree, points.begin(), points.end());
    quadtree.print();

    return 0;
}

/*
cardinal points
    N
W       E   
    S
*/