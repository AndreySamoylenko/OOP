#include <iostream>
#include <stdexcept>
#include "mine_vector.hpp"

using namespace std;

template <typename T>
ostream &operator<<(std::ostream &os, const Vector<T> vect)
{
    for (auto it = vect.begin(); it != vect.end(); ++it)
        os << *it << " ";
    return os;
}

template <typename T>
void print_filtered(const Vector<T> vector, const T value)
{
    for (auto it = vector.f_begin(value); it != vector.f_end(value); ++it)
        cout << *it << " ";
    cout << endl;
}

int main()
{
    Vector<int> str;

    // cout << str.empty() << endl;

    str.push_back(1);
    str.push_back(1);
    str.push_back(3);
    str.push_back(1);
    str.push_back(1);
    str.insert(99, 6);
    str.insert(99, 6);
    str.insert(99, 6);
    str.insert(-9, 3);
    str.insert(-9, 4);
    str.insert(-9, 0);
    str.push_back(1);

    cout << str.size() << endl;
    cout << str << endl;

    str.insert(4, 5);
    cout << str.size() << endl;

    cout << str << endl;
    print_filtered(str, 99);
    print_filtered(str, 1);
    print_filtered(str, -9);

    // cout << str.empty() << endl;
    // cout << str.pop_back() << endl;
    // cout << str.pop_back() << endl;
    // cout << str.pop_front() << endl;
    // cout << str.pop_back() << endl;
    // cout << str.pop_front() << endl;
    // cout << str.pop_back() << endl;

    return 0;
}