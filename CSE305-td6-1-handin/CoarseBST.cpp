class CoarseNode {
public:
    long key;
    CoarseNode* left;
    CoarseNode* right;
    CoarseNode* parent;
    CoarseNode() {}
    CoarseNode(long k) {
        this->key = k;
        this->left = NULL;
        this->right = NULL;
        this->parent = NULL;
    }
};

class CoarseBST {
protected:
    std::mutex lock;
    CoarseNode* root;
    static const unsigned long LOWEST_KEY = LONG_MIN;
    // returns the node that either contains k or would be 
    // a paretn is k is inserted
    static CoarseNode* search(CoarseNode* root, long k);
    // removes the given node
    static void remove_node(CoarseNode* n);
public:
    CoarseBST() {
        this->root = new CoarseNode(CoarseBST::LOWEST_KEY);
    }
    ~CoarseBST();
    bool add(long k);
    bool remove(long k);
    bool contains(long k);
};

void DeleteTree(CoarseNode* root) {
    if (root->left != NULL) {
        DeleteTree(root->left);
    }
    if (root->right != NULL) {
        DeleteTree(root->right);
    }
    delete root;
}

CoarseBST::~CoarseBST() {
    std::lock_guard<std::mutex> lk(lock);
    DeleteTree(this->root);
}

CoarseNode* CoarseBST::search(CoarseNode* root, long k) {
    CoarseNode* cur = root;
    while ((k > cur->key && cur->right != NULL) || (k < cur->key && cur->left != NULL)) {
        if (k > cur->key) {
	    cur = cur->right;
	} else {
	    cur = cur->left;
	}
    }
    return cur;
}

bool CoarseBST::contains(long k) {
    std::lock_guard<std::mutex> lk(this->lock);
    CoarseNode* cur = CoarseBST::search(this->root, k);
    return cur->key == k;
}

bool CoarseBST::add(long k) {
    std::lock_guard<std::mutex> lk(this->lock);
    CoarseNode* cur = CoarseBST::search(this->root, k);
    if (cur->key == k) {
        return false;
    }
    CoarseNode* new_node = new CoarseNode(k);
    new_node->parent = cur;
    if (k < cur->key) {
        cur->left = new_node;
    } else {
        cur->right = new_node;
    }
    return true;
}

void CoarseBST::remove_node(CoarseNode* n) {
    if (n->left == NULL || n->right == NULL) {
        CoarseNode* replacement = NULL;
        if (n->left != NULL) {
            replacement = n->left;
        }
        if (n->right != NULL) {
            replacement = n->right;
        }
        if (replacement != NULL) {
            replacement->parent = n->parent;
        }
        if (n->parent->left == n) {
            n->parent->left = replacement;
        } else {
            n->parent->right = replacement;
        }
        delete n;
    } else {
        CoarseNode* replacement = CoarseBST::search(n->right, n->key);
        n->key = replacement->key;
        CoarseBST::remove_node(replacement);
    }
}

bool CoarseBST::remove(long k) {
    std::lock_guard<std::mutex> lk(this->lock);
    CoarseNode* cur = CoarseBST::search(this->root, k);
    if (cur->key != k) {
        return false;
    }
    CoarseBST::remove_node(cur);
    return true;
}
