//
// Created by 陈燊 on 2021/11/23.
//

#ifndef MY_STL_ALLOCATOR_H
#define MY_STL_ALLOCATOR_H
#include "construct.h"

/*
 * 头文件包含模版类allocator，空间配置器，用于管理内存分配，对象构造和析构
 * 其中申请和释放内存调用标准库的operator new 和 operator delete
 * 构造和析构调用construct.h中封装的各种构造和析构函数.
 */

namespace my_stl {
    template <class T>
    class allocator {
    public:
        typedef T           value_type;
        typedef T*          pointer;
        typedef const T*    const_pointer;
        typedef T&          reference;
        typedef const T&    const_reference;
        typedef size_t      size_type;
        typedef ptrdiff_t   difference_type;

    public:
        /* 分配内存空间 */
        static T* allocate();
        static T* allocate(size_type n);

        /* 释放内存 */
        static void deallocate(T *ptr);
        static void deallocate(T *ptr, size_type n);

        /* 构造对象 */
        static void construct(T *ptr);
        static void construct(T *ptr, const T &value);
        static void construct(T *ptr, T &&value);

        /* 可变参数构造对象 */
        template <class ... Args>
        static void construct(T *ptr, Args &&...args);

        /* 析构对象 */
        static void destroy(T *ptr);
        static void destroy(T *first, T *last);
    };

    template <class T>
    T* allocator<T>::allocate() {
        return static_cast<T*>(::operator new(sizeof(T)));
    }

    template <class T>
    T* allocator<T>::allocate(size_type n) {
        /* 申请n个对象的内存空间 */
        if (n == 0)
            return nullptr;
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    template <class T>
    void allocator<T>::deallocate(T *ptr) {
        if (ptr == nullptr)
            return;
        ::operator delete(ptr);
    }

    template <class T>
    void allocator<T>::deallocate(T *ptr, size_type) {
        if (ptr == nullptr)
            return;
        ::operator delete(ptr);
    }

    template <class T>
    void allocator<T>::construct(T *ptr) {
        my_stl::construct(ptr);
    }

    template <class T>
    void allocator<T>::construct(T *ptr, T &&value) {
        my_stl::construct(ptr, my_stl::move(value));   //我认为用forward更好
    }

    template <class T>
    void allocator<T>::construct(T *ptr, const T &value) {
        my_stl::construct(ptr, value);
    }

    template <class T>
    template <class ...Args>
    void allocator<T>::construct(T *ptr, Args &&...args) {
        my_stl::construct(ptr, my_stl::forward<Args>(args)...);
    }

    template <class T>
    void allocator<T>::destroy(T *ptr) {
        my_stl::destroy(ptr);
    }

    template <class T>
    void allocator<T>::destroy(T *first, T *last) {
        /* 调用下层，下层会构造第三参数激活型别推导 */
        my_stl::destroy(first, last);
    }
}


#endif //MY_STL_ALLOCATOR_H
