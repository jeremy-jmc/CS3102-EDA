#include <bits/stdc++.h>
using namespace std;

class Point 
{
public:
    Point(double x = 0, double y = 0) : x(x), y(y) {}

    Point operator+(const Point& other) const { return Point(x + other.x, y + other.y); }

    Point operator-(const Point& other) const { return Point(x - other.x, y - other.y); }

    Point operator/(double divisor) const { return Point(x / divisor, y / divisor); }

    Point operator*(double factor) const { return Point(x * factor, y * factor); }

    Point operator-() const { return Point(-x, -y); }

    double getX() const { return x; }

    double getY() const { return y; }

private:
    double x;
    double y;
};

std::ostream& operator<<(std::ostream& os, const Point& p) 
{
    os << "(" << fixed << setprecision(3) << p.getX() << ", " << setprecision(3) << p.getY() << ")";
    return os;
}

class HilbertCurve {
public:
    HilbertCurve(int n) : n(n) 
    {
        counter = 0;
        path.resize(static_cast<size_t>(std::pow(4, n)));
    }

    std::vector<Point> getPoints() { return path; }

    void run() { hilbertCurve(Point(0,0), Point(1, 0), Point(0, 1), n); }

private:
    int n;
    int counter;
    std::vector<Point> path;

    void hilbertCurve(Point C, Point x, Point y, int n) 
    {
        double xx = C.getX(), yy = C.getY();
        double xi = x.getX(), xj = x.getY();
        double yi = y.getX(), yj = y.getY();

        for (int i = 1; i <= this->n - n; i++)
            cout << "\t";
        cout << " origen: " << C << "\t\teje X: " << x << "\t\teje Y: " << y << " n: " << n << endl;
        if (n == 0)
        {
            path[counter] = Point(xx + (xi + yi)/2, yy + (xj + yj)/2);
            counter++;
        }
        else 
        {
            hilbertCurve(Point(xx, yy), Point(yi/2, yj/2), Point(xi/2, xj/2), n-1);
            hilbertCurve(Point(xx + xi/2, yy + xj/2), Point(xi/2, xj/2), Point(yi/2, yj/2), n-1);
            hilbertCurve(Point(xx+xi/2+yi/2, yy+xj/2+yj/2), Point(xi/2, xj/2), Point(yi/2, yj/2), n-1);
            hilbertCurve(Point(xx+xi/2+yi, yy+xj/2+yj), Point(-yi/2, -yj/2), Point(-xi/2, -xj/2), n-1);
        }
    }
};

int main()
{
#ifndef ONLINE_JUDGE
    freopen("input.out", "r", stdin);
    freopen("output.out", "w", stdout);
#endif
    auto hc = HilbertCurve(3);
    hc.run();
    // auto points = hc.getPoints();
    // for (auto p : points)
    //     cout << p.getX() << " " << p.getY() << endl;
    return 0;
}