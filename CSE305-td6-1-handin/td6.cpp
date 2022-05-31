#include <algorithm>
#include <atomic>
#include <climits>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

//-----------------------------------------------------------------------------

class Node {
public:
    long key;
    std::mutex lock;
    Node* left;
    Node* right;
    Node* parent;
    Node() {}
    Node(long k) {
        this->key = k;
        this->left = NULL;
        this->right = NULL;
        this->parent = NULL;
    }
};

class FineBST {
protected:
    Node* root;
    static const unsigned long LOWEST_KEY = LONG_MIN;
    // returns the node that contains k or a node that would be
    // a parent of k if it will be inserted
    // Note: after the execution the returned node and its parent should be locked
    static Node* search(Node* root, long k);
    // removes the given node. Works under the assumption that the node and its parent
    // are locked
    static void remove_node(Node* n);
public:
    FineBST() {
        this->root = new Node(FineBST::LOWEST_KEY);
    }
    ~FineBST();
    bool add(long k);
    bool remove(long k);
    bool contains(long k);
};

void DeleteTree(Node* root) {
    std::lock_guard<std::mutex> lk(root->lock);
    if (root->left != NULL) {
        DeleteTree(root->left);
    }
    if (root->right != NULL) {
        DeleteTree(root->right);
    }
    delete root;
}

FineBST::~FineBST() {
    DeleteTree(this->root);
}

Node* FineBST::search(Node* root, long k) {
    root->lock.lock();
    Node *pred,*cur;
    cur = root;
    pred = nullptr;
    while ((k > cur->key && cur->right != NULL) ||
     (k < cur->key && cur->left != NULL)) {
        pred = cur;
        if (k > cur->key) {
	        cur = cur->right;
        }
        else {
            cur = cur->left;
        }
        cur->lock.lock();
        pred->lock.unlock();
    }
    return cur;
}

bool FineBST::contains(long k) {
    Node* cur = FineBST::search(this->root, k);
    cur->lock.unlock();
    return cur->key == k;
}

bool FineBST::add(long k) {
    Node* cur = FineBST::search(this->root, k);
    if (cur->key == k) {
        cur->lock.unlock();
        return false;
    }
    Node* new_node = new Node(k);
    new_node->parent = cur;
    if (k < cur->key) {
        cur->left = new_node;
    } else {
        cur->right = new_node;
    }
    cur->lock.unlock();
    return true;
}

void FineBST::remove_node(Node* n) {
    if (n->left == NULL || n->right == NULL) {
        Node* replacement = NULL;
        if (n->left != NULL) {
            replacement = n->left;
        }
        if (n->right != NULL) {
            replacement = n->right;
        }
        if (replacement != NULL) {
            // replacement->lock.lock();
            replacement->parent = n->parent;
        }
        if (n->parent->left == n) {
            n->parent->left = replacement;
        } else {
            n->parent->right = replacement;
        }
        // if(replacement!=NULL){
        //     replacement->lock.lock();
        // }
        // n->lock.unlock();
        delete n;
        // if(replacement!=NULL){
        //     replacement->lock.unlock();
        // }
    } else {
        Node* replacement = FineBST::search(n->right, n->key);
        // replacement->lock.lock();
        n->key = replacement->key;
        // n->lock.unlock();
        FineBST::remove_node(replacement);
        // replacement->lock.unlock();
    }

    n->lock.unlock();
}

bool FineBST::remove(long k) {
    Node* cur = FineBST::search(this->root, k);
    bool ret = true;
    if (cur->key != k) {
        ret = false;
        cur->lock.unlock();
        return ret;
    }
    FineBST::remove_node(cur);
    // cur->lock.unlock();
    return ret;
}