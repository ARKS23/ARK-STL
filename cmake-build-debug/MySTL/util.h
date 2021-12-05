//
// Created by 陈燊 on 2021/11/23.
//

#ifndef MY_STL_UTIL_H
#define MY_STL_UTIL_H
#include <stddef.h>
#include "type_traits.h"

/*
 * Line 66 可能存在bug
 */

/*
 * 引用折叠规则：
 * X& + & => X&
 * X&& + & => X&
 * X& + && => X&
 * X&& + && => X&&
 * 引用折叠发生在:模板实例化，auto型别生成,创建和运用typedef和别名声明.
 */

namespace my_stl {
    /* move */
    /* 实现过程, 先去引用，再转成右值引用 */
    template <class T>
    typename std::remove_reference<T>::type&& move(T &&arg) noexcept {
        return static_cast<typename std::remove_reference<T>::type&&>(arg);
    }

    /* -------------------------forward 完美转发实现 -------------------------- */
    /*
     * 当t是左值引用时，此时推到得到T = X&，则T&&展开为X& &&，
     * 经过引用折叠后得到X&，即最后返回static_cast<X&>(t)。
     */
    template <class T>
    T&& forward(typename std::remove_reference<T>::type &arg) noexcept {
        return static_cast<T&&>(arg);
    }

    /*
     * 当t是右值引用时，此时推到得到T = X&&，则T&&展开为X&& &&，
     * 经过引用折叠后得到X&&，即最后返回static_cast<X&&>(t)
     */
    template <class T>
    T&& forward(typename std::remove_reference<T>::type &&arg) noexcept {
        static_assert(!std::is_lvalue_reference<T>::value, "bad forward");
        return static_cast<T&&>(arg);
    }

    /* swap */
    template <class T>
    void swap(T &lhs, T &rhs) {
        /* 移动构造 */
        auto temp(my_stl::move(lhs));
        lhs = my_stl::move(rhs);
        rhs = my_stl::move(temp);
    }

    /* 前范围区间换到后面的开始区间 */
    template <class ForwardIter_1, class ForwardIter_2>
    ForwardIter_2 swap_range(ForwardIter_1 first1, ForwardIter_2 last1, ForwardIter_2 first2) {
        while (first1 != last1) {
            my_stl::swap(*first1, *first2);
            first1++;
            first2++;                //此处和MyTinySTL相比没进行转化，可能出现bug
        }
        return first2;
    }

    template <class T, size_t N>
    void swap(T (&a)[N], T(&b)[N]) {
        my_stl::swap_range(a, a + N, b);
    }

    /* ------------------------------pair数据结构实现----------------------------*/
    /*
     * 跟标准库中的pair保持一致性: first表示第一个数据,second表示第二个数据
     */
    template <class Ty1, class Ty2>
    struct pair {
        typedef Ty1 first_type;
        typedef Ty2 second_type;

        first_type first;
        second_type  second;

        // default constructable, 检查是否默认可构造
        template <class Other1 = Ty1, class Other2 = Ty2,
                typename = typename std::enable_if<
                        std::is_default_constructible<Other1>::value &&
                        std::is_default_constructible<Other2>::value, void>::type>
        constexpr pair() : first(), second() {}

        // implicit constructable for this type
        template <class U1 = Ty1, class U2 = Ty2,
                typename std::enable_if<
                        std::is_copy_constructible<U1>::value &&
                        std::is_copy_constructible<U2>::value &&
                        std::is_convertible<const U1&, Ty1>::value &&
                        std::is_convertible<const U2&, Ty2>::value, int>::type = 0>
        constexpr pair(const Ty1& a, const Ty2& b) : first(a), second(b) {}

        // explicit constructable for this type
        template <class U1 = Ty1, class U2 = Ty2,
                typename std::enable_if<
                        std::is_copy_constructible<U1>::value &&
                        std::is_copy_constructible<U2>::value &&
                        (!std::is_convertible<const U1&, Ty1>::value ||
                         !std::is_convertible<const U2&, Ty2>::value), int>::type = 0>
        explicit constexpr pair(const Ty1& a, const Ty2& b) : first(a), second(b) {}

        pair(const pair &rhs) = default;
        pair(pair &&rhs) = default;

        // implicit constructable for other type
        template <class Other1, class Other2,
                typename std::enable_if<
                        std::is_constructible<Ty1, Other1>::value &&
                        std::is_constructible<Ty2, Other2>::value &&
                        std::is_convertible<Other1&&, Ty1>::value &&
                        std::is_convertible<Other2&&, Ty2>::value, int>::type = 0>
        constexpr pair(Other1&& a, Other2&& b)
                : first(my_stl::forward<Other1>(a)),
                  second(my_stl::forward<Other2>(b)) {}

        // explicit constructable for other type
        template <class Other1, class Other2,
                typename std::enable_if<
                        std::is_constructible<Ty1, Other1>::value &&
                        std::is_constructible<Ty2, Other2>::value &&
                        (!std::is_convertible<Other1, Ty1>::value ||
                         !std::is_convertible<Other2, Ty2>::value), int>::type = 0>
        explicit constexpr pair(Other1&& a, Other2&& b)
                : first(my_stl::forward<Other1>(a)),
                  second(my_stl::forward<Other2>(b)) {}

        // implicit constructable for other pair
        template <class Other1, class Other2,
                typename std::enable_if<
                        std::is_constructible<Ty1, const Other1&>::value &&
                        std::is_constructible<Ty2, const Other2&>::value &&
                        std::is_convertible<const Other1&, Ty1>::value &&
                        std::is_convertible<const Other2&, Ty2>::value, int>::type = 0>
        constexpr pair(const pair<Other1, Other2>& other)
                : first(other.first),
                  second(other.second) {}

        // explicit constructable for other pair
        template <class Other1, class Other2,
                typename std::enable_if<
                        std::is_constructible<Ty1, const Other1&>::value &&
                        std::is_constructible<Ty2, const Other2&>::value &&
                        (!std::is_convertible<const Other1&, Ty1>::value ||
                         !std::is_convertible<const Other2&, Ty2>::value), int>::type = 0>
        explicit constexpr pair(const pair<Other1, Other2>& other)
                : first(other.first),
                  second(other.second) {}

        // implicit constructable for other pair
        template <class Other1, class Other2,
                typename std::enable_if<
                        std::is_constructible<Ty1, Other1>::value &&
                        std::is_constructible<Ty2, Other2>::value &&
                        std::is_convertible<Other1, Ty1>::value &&
                        std::is_convertible<Other2, Ty2>::value, int>::type = 0>
        constexpr pair(pair<Other1, Other2>&& other)
                : first(my_stl::forward<Other1>(other.first)),
                  second(my_stl::forward<Other2>(other.second)) {}

        // explicit constructable for other pair
        template <class Other1, class Other2,
                typename std::enable_if<
                        std::is_constructible<Ty1, Other1>::value &&
                        std::is_constructible<Ty2, Other2>::value &&
                        (!std::is_convertible<Other1, Ty1>::value ||
                         !std::is_convertible<Other2, Ty2>::value), int>::type = 0>
        explicit constexpr pair(pair<Other1, Other2>&& other)
                : first(my_stl::forward<Other1>(other.first)),
                  second(my_stl::forward<Other2>(other.second)) {}

        /* 赋值运算符 */
        pair& operator=(const pair &rhs) {
            if (this != &rhs) {
                first = rhs.first;
                second = rhs.second;
            }
            return *this;
        }

        /* 移动赋值 */
        pair& operator=(pair &&rhs) {
            if (this != &rhs) {
                first = my_stl::move(rhs.first);
                second = my_stl::move(rhs.second);
            }
            return *this;
        }

        /* 其他pair的赋值运算 */
        template <class Other1, class Other2>
        pair& operator=(const pair<Other1, Other2> &other) {
            first = other.first;
            second = other.second;
            return *this;
        }

        /* 其他pair的移动赋值运算版本 */
        template <class Other1, class Other2>
        pair& operator=(pair<Other1, Other2> &&other) {
            first = my_stl::move(other.first);
            second = my_stl::move(other.second);
            return *this;
        }

        ~pair() = default;

        void swap(pair &other) {
            if (this != other) {
                my_stl::swap(first, other.first);
                my_stl::swap(second, other.second);
            }
        }
    };

    // 重载比较操作符
    template <class Ty1, class Ty2>
    bool operator==(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
    {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }

    template <class Ty1, class Ty2>
    bool operator<(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
    {
        return lhs.first < rhs.first || (lhs.first == rhs.first && lhs.second < rhs.second);
    }

    template <class Ty1, class Ty2>
    bool operator!=(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
    {
        return !(lhs == rhs);
    }

    template <class Ty1, class Ty2>
    bool operator>(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
    {
        return rhs < lhs;
    }

    template <class Ty1, class Ty2>
    bool operator<=(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
    {
        return !(rhs < lhs);
    }

    template <class Ty1, class Ty2>
    bool operator>=(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
    {
        return !(lhs < rhs);
    }

    /* 重载swap */
    template <class Ty1, class Ty2>
    void swap(pair<Ty1, Ty2> &lhs, pair<Ty1, Ty2> &rhs) {
        lhs.swap(rhs);
    }

    /* 全局函数make_pair */
    template <class Ty1, class Ty2>
    pair<Ty1, Ty2> make_pair(Ty1 &&first, Ty2 &&second) {
        /* 使用完美转发，转发到构造函数的参数 */
        return pair<Ty1, Ty2>(my_stl::forward<Ty1>(first), my_stl::forward<Ty2>(second));
    }
}


#endif //MY_STL_UTIL_H
