//i guess here is where the magic will happen
#include <iostream>
#include <vector>
#include "bst.hpp"
#include "min-heap.hpp"

using namespace std;

int main() {
    int min = 1;
    int max = 1500;
    vector<int> rvec;
    for(int =0; i < 1000; i++){
        int random_number = min + rand() % ((max - min) +1);
        rvec.push_back(random_number);
    }
    while(true) {
        int k;
        cout <<"1. Populate values to BST"<<endl;
        cout <<"2. Populate values to min-heap"<<endl;
        cout <<"3. Sort desc using BST and print the first 20 values"<<endl;
        cout <<"4. Sort desc using min-heap and print the first 20 values"<<endl;
        cout <<"5. Print Summary"<<endl;
        cout <<"6. Exit"<<endl;
        cout <<"Enter your choice (value between 1-6):"<<endl;
        cin>>k;
        if(k == 6) {
            break;
        } else if (k == 5) {
            //print summary
        } else if (k == 4) {
            //min-heap
        } else if (k == 3) {
            //bst print
        } else if (k == 2) {
            //min-heap populate
        } else if (k == 1) {
            //bst populate
        } else {
            //default
        }
    }
    return 0;
}
