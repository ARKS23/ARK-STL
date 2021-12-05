//
// Created by 陈燊 on 2021/11/26.
//

#ifndef MY_STL_MYMEMORY_H
#define MY_STL_MYMEMORY_H

#include <cstddef>
#include <cstdio>
#include <climits>
#include "algobase.h"
#include "allocator.h"
#include "construct.h"
#include "uninitialized.h"

namespace my_stl {
    /* 获取地址 */
    template <class T>
    constexpr T* address_of(T &value) noexcept {
        return &value;
    }

    /* 获取内存空间 */
    template <class T>
    pair<T*, ptrdiff_t> get_buffer_helper(ptrdiff_t len, T*) {
        if (len > static_cast<ptrdiff_t>(INT_MAX / sizeof (T)))
            len = INT_MAX / sizeof(T);
        while (len > 0) {
            T *temp = static_cast<T*>(malloc(static_cast<size_t>(len) * sizeof(T)));
            if (temp)                               /* 申请内存成功的情况 */
                return pair<T*, ptrdiff_t>(temp, len);
            len /= 2;                               /* 申请失败的话减少再申请*/
        }
        return pair<T*, ptrdiff_t>(nullptr, 0);
    }

    template <class T>
    pair<T*, ptrdiff_t> get_temporary_buffer(ptrdiff_t len) {
        return get_buffer_helper(len, static_cast<T*>(0));
    }

    template <class T>
    pair<T*, ptrdiff_t> get_temporary_buffer(ptrdiff_t len, T*) {
        return get_buffer_helper(len, static_cast<T*>(0));
    }

    /* 释放内存空间 */
    template <class T>
    void release_temporary_buffer(T *ptr) {
        free(ptr);
    }

    /****************************************************************************************
     * 进行临时缓冲区申请与释放的类
     ****************************************************************************************/
     template <class ForwardIter, class T>
     class temporary_buffer {
     public:
         temporary_buffer(ForwardIter first, ForwardIter last);
         ~temporary_buffer() {my_stl::destroy(buffer, buffer + len); free(buffer);}

     public:
         ptrdiff_t size()           const noexcept {return len;}
         ptrdiff_t requested_size() const noexcept {return original_len;}
         T* begin()                 noexcept {return buffer;}
         T* end()                   noexcept {return buffer + len;}

     private:
         void allocate_buffer();
         void initialized_buffer(const T&, std::true_type) {}
         void initialized_buffer(const T& value, std::false_type) {my_stl::uninitialized_fill_n(buffer, len, value);}

     private:
         ptrdiff_t original_len; /* 缓冲区申请的大小 */
         ptrdiff_t len;          /* 缓冲区实际的大小 */
         T*        buffer;       /* 指向缓冲区的指针 */
     };

     template <class ForwardIter, class T>
     temporary_buffer<ForwardIter, T>::temporary_buffer(ForwardIter first, ForwardIter last) {
         try {
             len = my_stl::distance(first, last);   /* 计算地址长度 */
             allocate_buffer();
             if (len > 0)
                 initialized_buffer(*first, std::is_trivially_default_constructible<T>());
         } catch (...) {
             free(buffer);
             buffer = nullptr;
             len = 0;
         }
     }

     template <class ForwardIter, class T>
     void temporary_buffer<ForwardIter, T>::allocate_buffer() {
         original_len = len;                       /* 记录原始地址长度,因为后面可能会申请内存失败 */
         if (len > static_cast<ptrdiff_t>(INT_MAX / sizeof (T)))
             len = INT_MAX / sizeof(T);
         while (len > 0) {
             buffer = static_cast<T*>(malloc(len * sizeof(T)));
             if (buffer)
                 break;
             len /= 2;                          /* 申请失败，缩小空间再申请 */
         }
     }

     /*****************************************************************************************
      * 智能指针: my_ptr
      *****************************************************************************************/
      template <class T>
      class my_ptr {
      public:
        explicit my_ptr(T *p = nullptr) : m_ptr(p) {}
        my_ptr(my_ptr &rhs) : m_ptr(rhs.release()) {}
        template <class U> my_ptr(my_ptr<U> &rhs) : m_ptr(rhs.release()) {}
        my_ptr& operator=(my_ptr &rhs) {if (this != &rhs) {delete m_ptr;m_ptr = rhs.release();}return *this;}
        template <class U>
        my_ptr& operator=(my_ptr<U> &rhs) {if (this->get() != rhs.get()) {delete m_ptr;m_ptr = rhs.m_ptr;}return *this;}
        ~my_ptr() {delete m_ptr;}

      public:
          /* 获得指针 */
          T* get() {return m_ptr;}
          /* 释放指针 */
          T* release() {T *temp = m_ptr;m_ptr = nullptr;return temp;}
          /* 重置指针 */
          void reset(T *p = nullptr) {if (m_ptr != p) {delete m_ptr;m_ptr = p;}}
          /* 重载运算符 */
          T& operator*() const {return *m_ptr;}
          T* operator->() const {return m_ptr;}


      private:
          T *m_ptr;

      public:
          typedef T elem_type;
      };
}


#endif //MY_STL_MYMEMORY_H
