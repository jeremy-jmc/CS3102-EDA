#include <bits/stdc++.h>
using namespace std;

/* BTREE
root
internal nodes
leaves

    each node has n keys
    the keys are stored in increasing order
    internal node with n keys has n+1 children

    all the leaves have the same depth (height of a tree)
*/
const int t = 2, N = 2*t - 1;

template<typename T = int>
struct Node {
// attributes
    int n = 0;
    T* keys = new T[N];
    Node<T>** children = new Node<T>*[N+1]{};
    
    bool leaf = false;
    Node()
    {
        // ? why {} is not the same than `default` keyword, when did not used **
        for (int i=0; i<= N; i++)
        {
            if (children[i] == nullptr)
                cout<<"null"<<" ";
            else
                cout<<children[i]<<" ";
        }
    }

// methods
    bool add(T value)
    {
        if (n == N) return false;
        
    }
    bool is_empty(){ return n == 0; }
    bool is_full(){ return n == N; }
    bool is_leaf(){ return leaf; }
    int keys_stored(){ return n; }
};

template<typename T = int>
struct BTree {
private:
    Node<T>* root = nullptr;

    pair<Node<T>*, int> search_(Node<T>* node, T value)
    {
        // ? EDGE CASES: largest/smallest element

        int i = 0;
        int node_size = node->n;
        while (i < node_size && value > node->key[i])
            i++;
        
        if (i < node_size && value == node->key[i])
            return {node, i};
        else if (node->is_leaf())
            return {nullptr, -1};
        else
            // find in the right child   
            return search_(node->children[i], value);
    }
public:
    BTree()
    {
        root = new Node<T>();
        root->leaf = true;
        root->n = 0;
    }
    ~BTree() = default;

    bool search(T value)
    { 
        auto [node, index] = this->search_(root, value);
        if (node == nullptr)
        {
            throw std::logic_error("value not found\n");
            return false;
        }

        cout << node->keys[index] << endl;
        return true;
    }

    bool insert(T value)
    {
        return true;
    }

    // bool erase()
    // {

    // }
};

int main()
{
    int l=10, r=150;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(l, r);      // dist(gen);
    
    vector<int> numbers;
    for (size_t i = l; i <= r; i+=10)
        numbers.push_back(i);
    // random_shuffle(numbers.begin(), numbers.end());
    for (int n: numbers)
        cout << n << " ";
    cout << endl;
    
    // CRUD tests -> insert, search, delete
    auto bt = new BTree();

    // for (int n: numbers)
    //     bt->insert(n);
    


    return 0;
}

/* SOURCE
    Intro to Algorithms
    Michael Sambol YT
*/

/*
BTree

- Idea: force the tree to be perfect(balanced)
    - Problem: can't have an arbitrary # of nodes
    - Perfect binary trees only have 2^h - 1 nodes
- So: relax teh condition that the search tree be binary

- BTree: a tree of order m is a tree which satisfies the following properties:
    - Every node has at most m children
    - Every non-leaf node (except root) has at least ⌈m/2⌉ children
    - The root has at least two children if it is not a leaf node
    - A non-leaf node with k children contains k-1 keys
    - All leaves appear in the same level
    - A B-tree of order m is a m-ary tree

Insertion:
    - Start at the root
    - If the root is a leaf, insert the key
    - If the root is not a leaf, find the child to follow
    - If the child is not full, follow the child
    - If the child is full, split the child and then follow one of the two children
*/
