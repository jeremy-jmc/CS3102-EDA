#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <utility>
#include <tuple>

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
    os << "(" << p.getX() << ", " << p.getY() << ")";
    return os;
}


class CurveDrawer 
{
public:
    CurveDrawer(int width, int height) : window(sf::VideoMode(width, height), "Space filling curve") {}

    void draw(const std::vector<Point>& points) 
    {
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) 
            {
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            window.clear(sf::Color::Black);

            sf::VertexArray lines(sf::LineStrip, points.size());

            for (size_t i = 0; i < points.size(); ++i) 
            {
                const Point& point = points[i];
                lines[i].position = sf::Vector2f(point.getX(), point.getY());
                lines[i].color = sf::Color::Red;
            }

            window.draw(lines);
            window.display();
        }
    }

private:
    sf::RenderWindow window;
};


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
        if (n == 0)
        {
            path[counter] = Point(xx + (xi + yi)/2, yy + (xj + yj)/2);
            counter++;
            // path.push_back(Point(xx + (xi + yi)/2, yy + (xj + yj)/2));
            // for (Point& p : path)
            //     std::cout << " ("<< p.getX() << " " << p.getY() << ") ";
            // std::cout << std::endl;
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

int morton(int x, int y) 
{
    int z = 0;
    for (int i = 0; i < int(sizeof(int) * 8); ++i)
        z |= (x & (1 << i)) << i | (y & (1 << i)) << (i + 1);
    return z;
}


// Crear la clase para Z-curve
class ZCurve {
public:
    ZCurve(int n) : n(1<<n) {}

    std::vector<Point> getPoints() { return path; }

    void run() { zCurve(n); }
private:
    int n;
    int counter;
    std::vector<Point> path;

    void zCurve(int n)
    {
        std::vector<
            std::pair<int, std::pair<int, int>>
        > psum;
        for (int x = 0; x < n; x++)
        {
            for (int y = 0; y < n; y++)
            {
                
                auto sum = morton(x, y);
                // std::cout << x << " " << y << " -> " << sum << std::endl;
                psum.push_back({sum, {x, y}});
            }
        }
        sort(psum.begin(), psum.end(), [](std::pair<int, std::pair<int, int>> fp, std::pair<int, std::pair<int, int>> sp) 
        {
            auto [sf, pf] = fp;
            auto [ss, ps] = sp;
            
            return sf < ss;
        });

        for (auto [s, par] : psum) 
        {
            auto [x, y] = par;
            // std::cout << s << " {" << x << " " << y << "}\n";
            path.push_back(Point(x, y));
        }

        // std::cout << std::endl;
    }
};

// Crear la clase para Gray-curve

int to_gray_code(int n) {
    return n ^ (n >> 1);
}

int rev_g (int g) {
  int n = 0;
  for (; g; g >>= 1)
    n ^= g;
  return n;
}

class GrayCurve {
public:
    GrayCurve(int n) : n(1<<n) {}

    std::vector<Point> getPoints() { return path; }

    void run() { grayCurve(); }
private:
    int n;
    int counter;
    std::vector<Point> path;

    void grayCurve()
    {
        std::vector<
            std::pair<int, std::pair<int, int>>
        > psum;
        for (int x = 0; x < n; x++)
        {
            for (int y = 0; y < n; y++)
            {   
                auto sum = rev_g(morton(x, y));
                // std::cout << x << " " << y << " -> " << sum << std::endl;
                psum.push_back({sum, {x, y}});
            }
        }
        sort(psum.begin(), psum.end(), [](std::pair<int, std::pair<int, int>> fp, std::pair<int, std::pair<int, int>> sp) 
        {
            auto [sf, pf] = fp;
            auto [ss, ps] = sp;
            
            return sf < ss;
        });

        for (auto [s, par] : psum) 
        {
            auto [x, y] = par;
            // std::cout << s << " {" << x << " " << y << "}\n";
            path.push_back(Point(x, y));
        }
    }
};



int main() {
    int n = 2;
    GrayCurve curve(n);     // HilbertCurve, ZCurve
    curve.run();
    std::vector<Point> points = curve.getPoints();
    // for (Point& p : points)
    //     std::cout << p.getX() << " " << p.getY() << std::endl;

    int width  = 800;
    int height = 800;

    CurveDrawer drawer(width, height);
    double scaleFactor = (width * 0.02) ;       // 0.02, 0.5, 0.05
    for (auto& point : points)
        point = point * scaleFactor + Point(width * 0.05, height * 0.05);
    drawer.draw(points);

    return 0;
}
