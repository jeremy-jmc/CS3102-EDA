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
const int t = 3, N = 2*t - 1, width_output = 4;

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
        /*
        for (int i=0; i<= N; i++)
        {
            if (children[i] == nullptr)
                cout<<"null"<<" ";
            else
                cout<<children[i]<<" ";
        }
        */
    }

// methods
    bool add(T value)
    {
        if (n == N) return false;
        
    }
    bool is_empty(){ return n == 0; }
    bool is_full(){ return n == N; }
    bool is_leaf(){ return leaf; }
};

template<typename T = int>
ostream& operator<<(ostream& os, Node<T>* node)
{
    os << "[ ";
    if (node != nullptr)
    {
        for (int i = 0; i < N; i++)
        {
            // os << setw(width_output) << std::left;
            if (i < node->n)
                os << node->keys[i];
            else
                os << "x";
            os << " ";
        }
    }
    os << "]";
    return os;
}

template<typename T = int>
struct BTree {
public:
    Node<T>* root = nullptr;
    int height = 0;

    BTree()
    {
        root = new Node<T>();
        root->leaf = true;
        root->n = 0;
    }
    ~BTree() = default;

    bool search(T value)
    { 
        auto [node, index] = this->search_node(root, value);
        if (node == nullptr)
        {
            throw std::logic_error("value not found\n");
            return false;
        }

        cout << node->keys[index] << endl;
        return true;
    }

    void insert(T value)
    {
        // print(this->root);

        /*
        we insert the new key into an existing leaf node
        since we cannot insert a key into a leaf node that is full
            we introduce an operation that splits a full node y (having 2t-1 keys)
            around its median key y.key_t into two nodes having only t-1 keys each
            the median key moves up into y's parent is also full
            we must split it before we can insert the new key
            and thus we could en up splitting full nodes all the way up the tree

        the procedures uses split_child procedure to guarantee that the recursion never descends to a full node
        splitting the root is the only way to increase the height of a BTree
        Btree increases the  height at the top instead of the bottom.
        */
        Node<T>* r = this->root;
        if (r->is_full())
        {
            auto s = new Node<T>();
            s->leaf = false;
            s->n = 0;
            s->children[0] = r;

            split_child(s, 0);
            this->root = s;
            // print(this->root);
        }
        r = this->root;
        insert_non_full(r, value);
    }

private:
    /**
     * @param node node to insert the new key
     * @param value key to be inserted
    */
    void insert_non_full(Node<T>*& node, T value)
    {
        /*
        insert the key k into the tree rooted at the nonfull root node
        recurses as necessary downt the tree, 
            all the times guaranteeing that the node to which it recurses is not full 
            by calling split_child procedure as necessary
        inserts the key k into node x, which is assumed to be nonfull when the procedure is called
        insert and insert_non_full guarantee this assumption is true        
        */

        int i = node->n - 1;
        if (node->is_leaf())
        {
            // insert the new key due to the node have space
            while (i >= 0 and value < node->keys[i])
            {
                node->keys[i+1] = node->keys[i];
                i--;
            }
            node->keys[i+1] = value;
            node->n++;
        }
        else
        {
            while (i >= 0 and value < node->keys[i])
                i--;
            i++;
            if (node->children[i] == nullptr)
                node->children[i] = new Node<T>();
            
            if (node->children[i]->n == N)
            {
                // child node corresponding to the subtree where value will insert, is full
                split_child(node, i);
                // print(this->root);

                // compare value with the new median due to the split produces changes in the keys
                if (value > node->keys[i])     
                    i++;
            }
            insert_non_full(node->children[i], value);
        }
    }

    /**
     * @param X the parent node 
     * @param i the index of X_ci a full child of X
     * @return
    */
    void split_child(Node<T>*& X, int i)
    {
        /*
        y = X_ci
        median key S which moves up into y's parent node X
        those keys in y that are greater than the median key move into a new node z
        which becomes a new child of X

        disclaimer: always split the child when the node is full
        */
        // create the new right child Z
        auto Z = new Node<T>();
        auto Y = X->children[i];
        // cout << "\t-> " << X << " " << Y << " " << Z << endl;

        Z->leaf = Y->leaf;
        Z->n = t-1;
        for (int j = 0; j < t-1; j++)
            Z->keys[j] = Y->keys[j + t];

        // copy the corresponding children of Y to Z
        if (!Y->leaf)
        {
            for (int j = 0; j < t; j++)
                Z->children[j] = Y->children[j+t];
        }
        Y->n = t-1;
        // cout << "\t-> " << X << " " << Y << " " << Z << endl;

        // move children and keys one to the right
        // ! std::out_of_range if X.is_full()
        for (int j = X->n; j >= i; j--)
            X->children[j+1] = X->children[j];
        
        X->children[i+1] = Z;           // copy Z to right child of i-th key

        for (int j = X->n - 1; j >= i; j--)
            X->keys[j+1] = X->keys[j];
        
        X->keys[i] = Y->keys[t-1];          // copy the median from Y to X
        X->n++;
        // cout << "\t-> " << X << " " << Y << " " << Z << endl;
        this->height++;
    }

    /**
     * @param node the current node to search the value
     * @param value the key to be searched
     * @return an std::pair with the node and the index of the value, otherwhise {nullptr, -1}
    */
    pair<Node<T>*, int> search_node(Node<T>* node, T value)
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
            return search_node(node->children[i], value);
    }
};

template<typename T = int>
void print_aux(Node<T>* node, int level = 0)
{
    if (node != nullptr)
    {
        for (size_t i = 0; i < level; i++)
            cout << "\t";
        
        cout << node << endl;
        for (int i = node->n; i >= 0; i--)
            print_aux(node->children[i], level+1);
    }
}

template<typename T = int>
void print(Node<T>* node_root)
{
    string str = "=============================================================";
    cout << str << endl;
    print_aux(node_root, 0);
    cout << str << endl;
}

int main()
{
    int l=10, r=1000;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(l, r);      // dist(gen);
    
    vector<int> numbers;
    for (int i = l; i <= r; i+=10)
        numbers.push_back(i);
    random_shuffle(numbers.begin(), numbers.end());

    
    for (int n: numbers)
        cout << n << " ";
    cout << endl;
    
    // CRUD tests -> insert, search, delete
    auto bt = new BTree();
    /*
    for (int i = l; i <= r; i+=10)
    {
        cout << "\n\n\n\tinsert(" << i << ")\n";
        bt->insert(i);
        print(bt->root);
    }
    */
        
    
    for (int n: numbers)
    {
        bt->insert(n);
        // cout << "\n\n\n\tinsert(" << n << ")\n";
        // print(bt->root);
    }
    print(bt->root);
    
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
