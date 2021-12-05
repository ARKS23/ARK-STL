#include <iostream>
#include <vector>
#include <list>
#include "cmake-build-debug/MySTL/type_traits.h"
#include "cmake-build-debug/MySTL/vector.h"
#include "cmake-build-debug/MySTL/functional.h"
#include "cmake-build-debug/MySTL/list.h"


using namespace std;

void test() {
    std::cout << "Hello, World!" << std::endl;
    int a = 20, b = 10;
    a ^= b; b ^= a; a ^= b;
    cout << "a = " << a << " b = " << b << endl;
    int n = 10;
    n = -~n;
    cout << "n = "<< n << endl;
    n = ~-n;
    cout << "n = " << n << endl;
}

void test_vector() {
    std::cout << "[===============================================================]\n";
    std::cout << "[----------------- Run container test : vector -----------------]\n";
    std::cout << "[-------------------------- API test ---------------------------]\n";
    int a[] = { 1,2,3,4,5 };
    my_stl::vector<int> v1;
    v1.print();
    my_stl::vector<int> v2(10);
    v2.print();
    my_stl::vector<int> v3(10, 1);
    v3.print();
    my_stl::vector<int> v4(a, a + 5);
    v4.print();
    my_stl::vector<int> v5(v2);
    v5.print();
    my_stl::vector<int> v6(std::move(v2));
    v6.print();
    my_stl::vector<int> v8, v9, v10;
    v8 = v3;
    v8.print();
    v9 = std::move(v3);
    v9.print();
    v10 = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    v10.print();

    cout << "------------------------------------Shrink to fit ---------------------------\n";
    for (int i = 10; i < 20; ++i) v10.push_back(i * 2);
    cout << "v10.size: " << v10.size() <<  "  v10.capacity: " << v10.capacity() << endl;
    v10.shrink_to_fit();
    cout << "v10.size: " << v10.size() <<  "  v10.capacity: " << v10.capacity() << endl;

    cout << "------------------------------------insert------------------------------------\n";
    auto it = v10.begin();
    v10.insert(it, 5, 10);
    v10.print();

    cout << "------------------------------------insert------------------------------------\n";
    v10.erase(v10.begin(), v10.begin() + 2); cout << endl;
    v10.print();
    v10.insert(v10.end(), 777); cout << endl;
    v10.print();
    v10.reverse();
    cout << "reverse" << endl;
    v10.print();
    v10.insert(v10.end() - 5, -10); cout << endl;
    v10.print();
    v10.clear();
    cout << "v10.size: " << v10.size() << endl;
    v10.print();

    cout << "------------------------------------erase------------------------------------\n";
    v10.assign(7, 5);
    v10.push_back(-1);
    v10.erase(v10.begin(), v10.end() - 1);
    v10.print();
    v10.erase(v10.end() - 1);
    v10.print();

    cout << "------------------------------------swap------------------------------------\n";
    v10.clear();
    v10.swap(v3);
    cout << "v4: ";v4.print();
    cout << "v10:"; v10.print();

    cout << "--------------------------emplace emplace_back----------------------------------\n";
    my_stl::vector<int> v_;
    for (int i = 100; i < 120; ++i) v_.emplace_back(i);
    v_.emplace(v_.begin() + 10, 300);
    v_.print();
    v_.clear();
    v_.print();
    /* emplace_back arg....*/
    my_stl::vector<my_stl::vector<int>> v;
    v.emplace_back(4, 10);
    v.emplace_back(5, 101);
    v.back().print();

    cout << "-------------------------- out ----------------------------------------------------\n";
    cout << v4 << endl;
    cout << v << endl;
}

void test_hash() {
    my_stl::hash<float> h;
    auto result = h(20.6);
    cout << "20.5's hash_code: " << result << endl;
    result = h(20);
    cout << "20's hash_code" << result << endl;
}

void test_list() {
    auto test_list = [](string msg, my_stl::list<int> &l) {
        std::cout << msg << ": " << l;
    };
    std::cout << "******************************构造测试******************************" << std::endl;
    my_stl::list<int> l1;
    std::cout << "默认构造: " << l1;
    my_stl::list<int> l2(5);
    std::cout << "单参构造: " << l2;
    my_stl::list<int> l3(5, 10);
    std::cout << "双参构造: " << l3;
    my_stl::list<int> l4({1, 2, 3, 4, 5});
    std::cout << "列表构造: " << l4;
    my_stl::list<int> l5(l4);
    std::cout << "拷贝构造: " << l5;
    my_stl::list<int> temp(4, -1);
    my_stl::list<int> l6(my_stl::move(temp));
    std::cout << "移动构造: " << l6;
    std::cout << "******************************测试通过******************************" << std::endl;

    std::cout << "******************************接口测试******************************" << std::endl;
    std::cout << "原 list: " << l4;
    std::cout << "empty: " << l4.empty() << std::endl;
    std::cout << "size: " << l4.size() << std::endl;
    std::cout << "max_size: " << l4.max_size() << std::endl;
    std::cout << "l4.front(): " << l4.front() << std::endl;
    std::cout << "l4.back(): " << l4.back() << std::endl;
    l4.push_back(100); std::cout << "l4.push_back(100): " << l4;
    l4.push_front(100); std::cout << "l4.push_front(100): " << l4;
    l4.pop_back();std::cout << "l4.pop_back(): " << l4;
    l4.pop_front();std::cout << "l4.pop_front(): " << l4;
    l4.emplace_back(10);test_list("l4.emplace_back(10)", l4);
    l4.emplace_front(10);test_list("l4.emplace_front(10)", l4);
    l4.insert(l4.begin(), -100);test_list("l4.insert(l4.begin(), -100)", l4);
    l4.emplace(l4.begin()++, 200);test_list("l4.emplace(l4.begin()++, 200)", l4);
    std::cout << "l6: " << l6;
    l4.swap(l6);std::cout << "l4.swap(l6)\n" << "l4:" << l4 << "l6: " << l6;
    l6.sort(); test_list("l6.sort()", l6);
    l6.sort(my_stl::greater<int>());test_list("l6.sort(my_stl::greater<int>()): ", l6);
    l6.unique();test_list("l6.unique(): ", l6);
    l6.reverse();test_list("l6.reverse(): ", l6);
    l6.erase(++++l6.begin());test_list("l6.erase(++++l6.begin()): ", l6);
    l6.erase(++++l6.begin(), ----l6.end());test_list("l6.erase(++++l6.begin(), ----l6.end()): ", l6);
    l6.clear(); test_list("l6.clear(): ", l6);
    std::cout << "******************************测试通过******************************" << std::endl;
}

int main() {
    test_list();
    return 0;
}

