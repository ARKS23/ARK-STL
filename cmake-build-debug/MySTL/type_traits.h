//
// Created by 陈燊 on 2021/11/20.
//

#ifndef MY_STL_TYPE_TRAITS_H
#define MY_STL_TYPE_TRAITS_H
// 此头文件用于提取类型信息
#include <type_traits>
namespace my_stl {
    template <class T, T v>
    struct m_integral_constant {
        static constexpr T value = v;
    };

    template <bool b>
    using m_bool_constant = m_integral_constant<bool, b>;
    typedef m_bool_constant<true> m_true_type;
    typedef m_bool_constant<false> m_false_type;

    template<class T1, class T2>
    struct pair;

    template <class T>
    struct is_pair : my_stl::m_false_type {};

    template <class T1, class T2>
    struct is_pair<my_stl::pair<T1, T2>> : my_stl::m_true_type {};
}




#endif //MY_STL_TYPE_TRAITS_H
