#include <iostream>
#include <vector>

using namespace std;

    
class Heap
{
private:
    vector<int> Arr, outVec;
    Heap();
    Heap(vector<int> arr, int c);
    vector<int> vec;

public:
    void insert_value_minHeap(int val); // Adds value at the end of array and calls min_heapify
    void min_heapify(int i);            // Maintains min heap property
    void descendingHeapSort();          // Prints the array in descending order
    void buildMinHeap();                // Heapifies entire list
};

void Heap::insert_value_minHeap(int val)
{
    int n, tmp;

    Arr.push_back(val);
    n = Arr.size() - 1;
    while ((n > 0) && (Arr[n] < Arr[(n - 1) / 2]))
    {
        tmp = Arr[n];
        Arr[n] = Arr[(n - 1) / 2];
        Arr[(n - 1) / 2] = tmp;
        n = (n - 1) / 2;
    }
}

void Heap::min_heapify(int i)
{
    int size = Arr.size();

    if (i < 0)
        return;

    int smallest, lIndex, rIndex;

    lIndex = 2 * i + 1;
    rIndex = 2 * i + 2;

    smallest = i;

    if (lIndex <= size - 1 && Arr[lIndex] < Arr[i])
    {
        smallest = lIndex;
    }
    if (rIndex <= size - 1 && Arr[rIndex] < Arr[smallest])
    {
        smallest = rIndex;
    }
    if (smallest != i)
    {

        int tmp = Arr[smallest];
        Arr[smallest] = Arr[i];
        Arr[i] = tmp;

        min_heapify(smallest);
    }
}

void Heap::buildMinHeap()
{

    for (int lastIndex = Arr.size() / 2 - 1; lastIndex >= 0; lastIndex--)
        min_heapify(lastIndex);
}

void Heap::descendingHeapSort()
{
    int tmp;
    vector<int> tmpVec;

    buildMinHeap();
    tmpVec = Arr;
    outVec.clear();

    while (!Arr.empty())
    {
        outVec.push_back(Arr[0]);
        tmp = Arr.back();
        Arr[0] = tmp;
        Arr.pop_back();
        buildMinHeap();
    }

    Arr = tmpVec;
}
