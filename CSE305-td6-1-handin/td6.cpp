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
    Node *pred,*cur;
    pred = root;
    pred->lock.lock();
    if(pred->key ==k) return pred;
    if(k>pred->key){
        if(!pred->right){
            return pred;
        }
        cur = pred->right;
    }
    else{
        if(!pred->left){
            return pred;
        }
        cur = pred->left;
    }
    cur->lock.lock();
    while ((k > cur->key && cur->right != NULL) ||
     (k < cur->key && cur->left != NULL)) {
        if (k > cur->key) {
	        pred->lock.unlock();
            cur->right->lock.lock();
            pred=cur;
            cur = cur->right;
        }
        else {
            pred->lock.unlock();
            cur->left->lock.lock();
            pred=cur;
            cur = cur->left;
        }
    }
    return cur;
}

bool FineBST::contains(long k) {
    Node* cur = FineBST::search(this->root, k);
    cur->lock.unlock();
    if (cur != this->root){
        cur->parent->lock.unlock();
    }
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
    if (cur != this->root){
        cur->parent->lock.unlock();
    }
    return true;
}

void FineBST::remove_node(Node* n) {
    if (n->left == NULL || n->right == NULL) {
        Node* replacement = NULL;
        if (n->left != NULL) {
            replacement = n->left;
        }
        else{
            replacement = n->right;
        }
        if (n->parent->left == n) {
            n->parent->left = replacement;
        }
        else {
            n->parent->right = replacement;

        }
        if (replacement != NULL) {
            replacement->parent = n->parent;
        }
        n->parent->lock.unlock();

        delete n;
    }
    else{
        Node* replacement = FineBST::search(n->right, n->key);
        n->key = replacement->key;
        n->parent->lock.unlock();
        FineBST::remove_node(replacement);
    }
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
    ret=true;
    // cur->lock.unlock();
    return ret;
}