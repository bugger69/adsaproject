//i guess here is where the magic will happen
#include <iostream>
#include <vector>

using namespace std;

int main() {
    int min = 1;
    int max = 1500;
    vector<int> rvec;
    for(int =0; i < 1000; i++){
        int random_number = min + rand() % ((max - min) +1);
        rvec.push_back(random_number);
    }
    return 0;
}
