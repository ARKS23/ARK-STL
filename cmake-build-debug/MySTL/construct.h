//
// Created by 陈燊 on 2021/11/20.
//

#ifndef MY_STL_CONSTRUCT_H
#define MY_STL_CONSTRUCT_H
#include <new>
#include "type_traits.h"
#include "iterator.h"
#include "util.h"

/* 此头文件负责对象的构造和析构 */

/* 参考《MyTinySTL》, 这个宏我不知道是干嘛的 */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#endif /* _MSC_VER */

namespace my_stl {
    /* construct 构造对象*/
    template <class T>
    void construct(T *ptr) {
        /* 调用的是placement_new, 定点默认构造 */
        ::new ( (void*)ptr ) T();
    }

    template <class T1, class T2>
    void construct(T1 *ptr, const T2 &value) {
        /* 调用placement_new，定点拷贝构造*/
        ::new ( (void*)ptr ) T1(value);
    }

    /* 可变参数构造 */
    template <class T, class... Args>
    void construct(T *ptr, Args&&... args) {
        /* C++11 新特性, 用到了参数模板, 万能引用和完美转发*/
        ::new( (void*)ptr ) T(my_stl::forward<Args>(args)...);
    }

    /* destroy 析构对象 : DOP型别和非DOP型别 */
    template <class T>
    void destroy_one(T*, std::true_type) {}

    /* 针对非POD型别的析构 */
    template <class T>
    void destroy_one(T *pointer, std::false_type) {
        if (pointer != nullptr) {
            pointer->~T();         /* 调用析构函数 */
        }
    }

    /* 上层调用函数 */
    template <class T>
    void destroy(T *pointer) {
        /* 第二参数激活推导 */
        destroy_one(pointer, std::is_trivially_destructible<T>{});
    }

    template <class ForwardIter>
    void destroy_cat(ForwardIter, ForwardIter, std::true_type) {}

    /* 针对非POD型别的析构 */
    template <class ForwardIter>
    void destroy_cat(ForwardIter first, ForwardIter last, std::false_type) {
        while (first != last) {
            destroy(&*first);      /* 不能直接析构迭代器，是析构迭代器里的内容*/
            first++;
        }
    }

    /* 上层调用 */
    template <class ForwardIter>
    void destroy(ForwardIter first, ForwardIter last) {
        /* 第三参数从萃取机中获得迭代器所指之物的型别，然后激活型别推导调用下层函数 */
        destroy_cat(first, last, std::is_trivially_destructible<
                typename iterator_traits<ForwardIter>::value_type>{});
    }
}

/* 参考《MyTinySTL》, 这个宏我不知道是干嘛的 */
#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif //MY_STL_CONSTRUCT_H
