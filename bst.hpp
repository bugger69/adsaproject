#include <iostream>
#include <vector>

using namespace std;

struct node{
    int data;
    node* left_child = NULL;
    node* right_child = NULL;
    bool flag = 0;
    node(){}
    node(int data){
        this->data = data;
    }
};

class BinarySearchTree{
    private:
        node* head_node = NULL;
        
        void inorder(node* &tnode, vector<int> &ivec){
            if(tnode->left_child == NULL && tnode->right_child == NULL){
                ivec.push_back(tnode->data);
                return;
            }
            if(tnode->left_child != NULL){
                inorder(tnode->left_child, ivec);
            }
            cout<<tnode->data<<" ";
            if(tnode->right_child != NULL){
                inorder(tnode->right_child, ivec);
            }
        }

        // void preorder(node* &tnode){
        //     if(tnode->left_child == NULL && tnode->right_child == NULL){
        //         cout<<tnode->data<<" ";
        //         return;
        //     }
        //     cout<<tnode->data<<" ";
        //     if(tnode->left_child != NULL){
        //         preorder(tnode->left_child);
        //     }
        //     if(tnode->right_child != NULL){
        //         preorder(tnode->right_child);
        //     }
        // }

        // void postorder(node* &tnode){
        //     if(tnode->left_child == NULL && tnode->right_child == NULL){
        //         cout<<tnode->data<<" ";
        //         return;
        //     }
        //     if(tnode->left_child != NULL){
        //         postorder(tnode->left_child);
        //     }
        //     if(tnode->right_child != NULL){
        //         postorder(tnode->right_child);
        //     }
        //     cout<<tnode->data<<" ";
        // }

        void insertion(int data, node* &tnode){
            if(tnode == NULL){
                node* n = new node(data);
                tnode = n;
            }
            if(data < tnode->data){
                if(tnode->left_child == NULL){
                    node* n = new node(data);
                    tnode->left_child = n;
                    return;
                }
                insertion(data, tnode->left_child);
            }
            else if(data > tnode->data){
                if(tnode->right_child == NULL){ 
                    node* n = new node(data);
                    tnode->right_child = n;
                    return;
                }
                insertion(data, tnode->right_child);
            }
        }

    public:
        void insert(int data){
            insertion(data, head_node);
        }

        // void postorder_traversal(){
        //     postorder(head_node);
        // }

        // void preorder_traversal(){
        //     preorder(head_node);
        // }
        
        vector<int> sort(){
            vector<int> ivec;
            inorder(head_node, ivec);
            return ivec;
        }
};
