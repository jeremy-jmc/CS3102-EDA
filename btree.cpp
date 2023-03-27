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

const int t = 2, N = 2*t - 1;        // * t: min_degree of a node = 2
// const double M = 4;                     // * M: tree order (min: 4)
// const int t = M/2, N = M - 1;
const int width_output = 4;

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
    bool is_empty(){ return n == 0; }
    bool is_full(){ return n == N; }
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
            // throw std::logic_error();
            cout << "value " << value << " not found\n";
            return false;
        }
        cout << "found " << node->keys[index] << "\n";
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

    void remove(T value)
    {
        remove_node(root, value);
    }
private:
    /**
     * deletes the key from the subtree rooted at X
     * @param
     * @param
    */
    void remove_node(Node<T>*& X, T value)
    {
        cout << __PRETTY_FUNCTION__ << endl;
        print(root);

        cout << "nodo actual: "<< X << "\t" << value << endl;

        if (X == nullptr)
        {
            cout << value << " not found\n";
            return;
        }
        /*
        we must guard against deletion producing a tree whose structure violates the properties
        we must ensure that a node does not get too small during deletion
        root is allowed to have fewer that the min number of keys (t-1)

        will guarantee that the number of keys in X is at least the min_degree t
        */
        int i = 0;
        bool found = false;
        for (; i < X->n && X->keys[i] <= value; i++)
        {
            if (X->keys[i] == value)
            {
                found = true;
                break;
            }
        }
        
        cout << i << boolalpha << " " << found << endl;

        if (found)
        {
            if (X->leaf)
            {   // ? leaf case: easy case
                cout << "\tleaf case\n";
                for (int j = i; j < X->n - 1; j++)
                    X->keys[j] = X->keys[j+1];
                X->n --;
            }
            else if (!X->leaf)
            {   // ? internal node case
                cout << "\tinternal node case\n";
                Node<T>* Y = X->children[i];
                if (Y->n >= t)
                {   // * try with left child Y
                    cout << "\t\treplace with predecessor\n";
                    T predecessor = Y->keys[Y->n - 1];     // k': last key in left children of X_i
                    X->keys[i] = predecessor;
                    remove_node(Y, predecessor);
                }
                else
                {   // Y has fewer than  n keys
                    // * try with the right child Z
                    Node<T>* Z = X->children[i + 1];
                    if (Z->n >= t)
                    {
                        cout << "\t\treplace with successor\n";
                        T succesor = Z->keys[0];     // k': first key of the right child of X_i
                        X->keys[i] = succesor;
                        remove_node(Z, succesor);
                    }
                    else
                    {   // both Y and Z have only t-1 keys
                        cout << "\t\tmerge\n";
                        // TODO: check merge operation
                        // merge Y and Z
                        // ! what happens to the children?
                        cout << Y << " " << Z << endl;
                        for (int j = 0; j < Z->n; j++)
                            Y->keys[j + Y->n] = Z->keys[j];
                        Y->n += Z->n;
                        cout << Y << endl;
                        delete Z, Z = nullptr;       // * release the node Z

                        // rearrange data of X, overlapping K
                        for (int j = i + 1; j < X->n; j++)
                        {
                            X->keys[j-1] = X->keys[j];
                            X->children[j] = X->children[j+1];
                        }
                        X->n--;
                    }
                }
            }
        }
        else
        {
            cout << " > traverse\n";
            i = X->n - 1;
            while (i >= 0 and value < X->keys[i])
                i--;
            // index such that value >= X.key[i]
            cout << i << endl;
            
            Node<T> *left_X_ci = nullptr, *right_X_ci = nullptr;
            Node<T> *X_ci = nullptr;
            if (i >= 0)
                X_ci = X->children[i];
            
            if (i > 0)
                left_X_ci = X->children[i-1];
            if (i < X->n)
                right_X_ci = X->children[i+1];
            
            // * rotation
            if (X_ci != nullptr && X_ci->n < t)
            {   // t-1 keys
                cout << "\trotation\n";
                Node<T>* Y = left_X_ci;
                Node<T>* Z = right_X_ci;
                
                cout << " Y -> " << Y << endl;
                cout << " X_ci -> " << X_ci << endl;
                cout << " Z -> " << Z << endl;
                if (left_X_ci != nullptr && left_X_ci->n >= t)
                {   // * move a key and the corresponding child from the left_sibbling to X
                    cout<<"\t\trotation from left\n";
                    Node<T>* W = Y->children[Y->n];     // last children

                    // * move keys and children one to the right to leave space to the rotation
                    for (int j = X->n; j >= 1; j++)
                        X->keys[j] = X->keys[j-1];
                    for (int j = X->n + 1; j >= 1; j--)
                        X->children[j] = X->children[j-1];
                    X_ci->n++;

                    // * perform rotation of keys
                    X_ci->keys[0] = X->keys[i];         // set new key from parent node
                    X->keys[i] = Y->keys[Y->n - 1];     // replace parent node key with the rightmost key of the left child/sibbling
                    
                    // * rearrange children
                    Y->children[Y->n] = nullptr;
                    X_ci->children[0] = W;
                    Y->n--;
                }   
                else if (right_X_ci && right_X_ci->n >= t)
                {   // * move a key and the corresponding child from the right_sibbling to X
                    cout<<"\t\trotation from right\n";
                    Node<T> *W = Z->children[0];        // first children
                    
                    // * perform rotation of the keys
                    X_ci->keys[X_ci->n] = X->keys[i];   // set new key from parent node
                    X->keys[i] = Z->keys[0];            // replace parent node key with the leftmost key of the left right/sibbling
                    
                    // * rearrange children
                    X_ci->children[X_ci->n + 1] = W;
                    X_ci->n++;
                    
                    // * move keys and children one to the left to fill the space of the rotation
                    for (int j = 0; j < Z->n; j++)
                    {
                        Z->keys[j] = Z->keys[j+1];
                        Z->children[j] = Z->children[j+1];
                    }
                    Z->children[Z->n] = nullptr;
                    Z->n--;
                }
                else
                {   // * X_ci's immediate sibblings have t-1 keys
                    // * merge X_ci with one sibbling (involves that the tree shrinks in height)
                    // * reduce children by one if X->n == 0
                    cout <<"\tmerge to the left\n";
                    // * merge X_ci with Y putting Xi as a median
                    cout << Y << " " << X << " " << X_ci << endl;
                    if (Y == nullptr)
                    {
                        Y = X_ci;
                        X_ci = Z;
                    }
                    cout << Y << " " << X << " " << X_ci << endl;
                    Y->keys[Y->n] = X->keys[i];
                    Y->children[Y->n + 1] = X_ci->children[0];
                    Y->n++;

                    for (int j = 0; j < X_ci->n; j++)
                    {
                        Y->keys[j + Y->n] = X_ci->keys[j];
                        Y->children[j + Y->n + 1] = X_ci->children[j + 1];
                    }
                    
                    // * move keys and children of X one to the left
                    for (int j = i; j < X->n; j++)
                        X->keys[i] = X->keys[i+1];
                    
                    for (int j = i + 1; j < X->n; j++)
                        X->children[j] = X->children[j+1];

                    X->n--;
                    // * reduce the height of a tree by one
                    if (X->n == 0)
                        X = X->children[0];
                }
            }
            int i = 0;
            bool found = false;
            for (; i < X->n && X->keys[i] <= value; i++)
            {
                if (X->keys[i] == value)
                {
                    found = true;
                    break;
                }
            }
            if (found)
                remove_node(X, value);    
            else
            {
                cout << X->children[i+1] << endl;
                remove_node(X->children[i+1], value);
            }
            
        }
    }

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
        if (node->leaf)
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
                // * child node corresponding to the subtree where value will insert, is full
                split_child(node, i);
                // print(this->root);

                // * compare value with the new median due to the split produces changes in the keys
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
        // * create the new right child Z
        auto Z = new Node<T>();
        auto Y = X->children[i];
        // cout << "\t-> " << X << " " << Y << " " << Z << endl;

        Z->leaf = Y->leaf;
        Z->n = t-1;
        for (int j = 0; j < t-1; j++)
            Z->keys[j] = Y->keys[j + t];

        // * copy the corresponding children of Y to Z
        if (!Y->leaf)
        {
            for (int j = 0; j < t; j++)
                Z->children[j] = Y->children[j+t];
        }
        Y->n = t-1;
        // cout << "\t-> " << X << " " << Y << " " << Z << endl;

        // * move children and keys one to the right
        // ! std::out_of_range if X.is_full()
        for (int j = X->n; j >= i; j--)
            X->children[j+1] = X->children[j];
        
        X->children[i+1] = Z;           // * copy Z to right child of i-th key
        
        // ! std::out_of_range if X.is_full()
        for (int j = X->n - 1; j >= i; j--)
            X->keys[j+1] = X->keys[j];
        
        X->keys[i] = Y->keys[t-1];      // * copy the median from Y to X
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
        while (i < node_size && value > node->keys[i])
            i++;
        
        if (i < node_size && value == node->keys[i])
            return {node, i};
        else if (node->leaf)
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
    // cout << t << " " << N << endl;
    int l=10, r=40;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(l, r);      // dist(gen);
    
    vector<int> numbers;
    for (int i = l; i <= r; i+=2)
        numbers.push_back(i);
    // random_shuffle(numbers.begin(), numbers.end());
    
    // for (int n: numbers)
    //     cout << n << " ";
    // cout << endl;
    
    // CRUD tests -> insert, search, delete
    auto bt = new BTree();
    
    for (int n: numbers)
    {
        // cout << "\tinsert(" << n << ")\n";
        bt->insert(n);
        // cout << "\n\n\n\tinsert(" << n << ")\n";
        // print(bt->root);
    }
    
    // print(bt->root);
    
    // for (int n: numbers)
    //     bt->search(dist(gen));
    
    // random_shuffle(numbers.begin(), numbers.end());
    vector<int> to_remove = {32, 36, 40, 10, 30, 20};
    for (int n: to_remove)
    {
        cout << "\tremove(" << n << ")\n";
        // print(bt->root);
        bt->remove(n);
    }
    print(bt->root);
    return 0;
}


/*
BTree

- Idea: force the tree to be perfect(balanced)
    - Problem: can't have an arbitrary # of nodes
    - Perfect binary trees only have 2^h - 1 nodes
- So: relax teh condition that the search tree be binary

- BTree: a tree of order m is a tree which satisfies the following properties:
    - Every node has at most m children
    - Every non-leaf node (except root) has at least ⌈m/2⌉ children (m/2 keys)
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
