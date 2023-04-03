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

void print_pair(const par &input)
{
    std::cout << input.first << "-" << input.second;
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
};

int correct_quad(const par &center, const par &new_point)
{
    if (center.first > new_point.first &&
        center.second <= new_point.second)
    { // For NW
        return NW;
    }
    else if (center.first <= new_point.first &&
             center.second <= new_point.second)
    {
        return NE;
    }
    else if (center.first > new_point.first &&
             center.second > new_point.second)
    { // For SW
        return SW;
    }
    else
    {
        return SE;
    }
}

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
};

class QuadTree
{
    Node *Root;
    friend void print(QuadTree &quadtree);

public:
    QuadTree(Node *new_root)
    {
        this->Root = new_root;
    }
    
    void insert(Node *new_node)
    {
        if (Root == nullptr)
        {
            Root = new_node;
            Root->leaf = true;
        }
        Node *aux = Root;

        while (aux != nullptr)
        {
            int quad = correct_quad(aux->point_coords, new_node->point_coords);
            if (aux->children[quad] != nullptr)
            {
                aux = aux->children[quad];
            }
            else
            {
                aux->children[quad] = new_node;
                aux->leaf = false;
                aux->children[quad]->leaf = true;
                break;
            }
        }
    }

    void find(const par &coord_point)
    {
    }

    bool remove(const par &coord_point)
    {
        Node *father = Root;
        Node *aux_iter = Root;
        int quad;
        while (aux_iter != nullptr &&
                   (aux_iter->point_coords).first != coord_point.first ||
               (aux_iter->point_coords).second != coord_point.second)
        {

            quad = correct_quad(aux_iter->point_coords, coord_point);
            father = aux_iter;
            aux_iter = aux_iter->children[quad];
        }

        if (aux_iter == nullptr)
        {
            return false;
        }
        else
        {
            if (aux_iter->leaf)
            {
                delete aux_iter;
                father->children[quad] = nullptr;
            }
        }
    }

    void print()
    {
        Node *Auxiliar;
        int Current_Depth = 0;
        // Contiene los nodos cuyos hijos todavia no se han tomado en cuenta
        std::queue<Node *> Children_Queue;
        std::queue<int> Children_Depth;
        std::queue<par> Children_Parent;

        // Imprimir la informacion del Root
        print_pair(this->Root->point_coords);
        std::cout << '\n';

        // Meter al Root en el Queue por que sus hijos no se han tomado en cuenta
        Children_Queue.push(this->Root);
        Children_Depth.push(0);
        Children_Parent.push({0, 0});
        // Mientras existan hijos que no se han tomado en cuenta
        while (!Children_Queue.empty())
        {
            // Tomar el primer hijo
            Auxiliar = Children_Queue.front();
            if (Current_Depth < Children_Depth.front())
            {
                std::cout << '\n';
                Current_Depth++;
            }

            // Recorrer los hijos del hijo
            for (int i = 0; i < 4; i++)
            {
                if (Auxiliar->children[i] != nullptr)
                {
                    // Imprimir la informacion del hijo del hijo
                    std::cout << "Parent:";
                    print_pair(Children_Parent.front());
                    std::cout << " ";
                    print_pair(Auxiliar->children[i]->point_coords);
                    std::cout << " ";

                    // Si el hijo del hijo no es un nodo hoja, añadirlo a la lista de hijos por recorrer
                    if (Auxiliar->children[i]->leaf == false)
                    {
                        Children_Queue.push(Auxiliar->children[i]);
                        Children_Depth.push(Children_Depth.front() + 1);
                        Children_Parent.push(Auxiliar->point_coords);
                    }
                }
            }

            // Eliminar el elemento del frente
            Children_Queue.pop();
            Children_Depth.pop();
            Children_Parent.pop();
        }
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
    print_aux(quadtree.Root);
}

int main()
{
#ifndef TEST
    // freopen("input.txt", "r", stdin);
    // freopen("output.txt", "w", stdout);
#endif

    time_t start, end;
    time(&start);
    int l = -20, r = 20;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(l, r); // dist(gen);

    Node *new_node = new Node;
    new_node->point_coords = {0, 0};
    QuadTree quadtree(new_node);

    for (int i = 0; i < 20; i++)
    {
        Node *another_node = new Node;
        another_node->point_coords = {dist(gen), dist(gen)};
        quadtree.insert(another_node);

        // cout << "================================================\n";
        // print(quadtree.Root);
        // cout << "================================================\n";
    }

    cout << "================================================\n";
    quadtree.print();
    cout << endl;
    cout << "================================================\n";
    print(quadtree);
    cout << "================================================\n";

    time(&end);
    double time_taken = double(end - start);
    cout << "\nTime: " << fixed << time_taken << " sec " << endl;
    // #ifdef LOCAL_TIME
    //     cout << "Time elapsed: " << 1.0 * clock() / CLOCKS_PER_SEC << "s\n";
    // #endif
}