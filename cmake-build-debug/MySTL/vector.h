//
// Created by 陈燊 on 2021/11/26.
//

#ifndef MY_STL_MY_VECTOR_H
#define MY_STL_MY_VECTOR_H
#include <initializer_list>
#include "iterator.h"
#include "mymemory.h"
#include "util.h"
#include "exceptdef.h"
#include <iostream>


/*
 * 平时最常用的vector类(向量)
 * 基本内部机制：vector是动态空间,随着元素的加入，它的内部机制会自行扩充空间以容纳新元素.
 * 迭代器:vector的迭代器类型其实就是原生指针,萃取机对原生指针有偏特化,是随机访问迭代器.可以实现相应的算法.
 *
 * 参考《MyTinySTL》
 *  部分函数异常安全保证:
 *      emplace()
 *      emplace_back()
 *      push_back()
 *  当std::is_nothrow_move_assignable<T>::value == true, 函数满足强异常保证:
 *      reserve
 *      resize
 *      insert
 *  添加了扩展成员函数:
 *      void print(const char *ends = " ") 默认用空格结尾，可根据用户喜好更改参数
 *  添加了拓展输出运算符:
 *      ostream& <<(ostream &os, const my_stl::vector<T> &vec); 方便输出向量内容
 */

/*
 * Line 253 可能存在bug
 * Line 432 可能存在bug,已更正
 * Line 393 可能存在bug
 */

namespace my_stl {
    /*参考《MyTinySTL》*/
#ifdef max
    #pragma message("#undefing marco max")
#undef max
#endif // max

#ifdef min
    #pragma message("#undefing marco min")
#undef min
#endif // min

    /* vector类 */
    template <class T>
    class vector {
        /* 暂时没有编写bool的vector, 因为标准库的vector<bool>做了特别的位优化 */
        static_assert(!std::is_same<bool, T>::value, "vector<bool> not in my_stl\n");

    public:
        /* 相关型别定义,vector的迭代器类型其实就是原生指针 */
        typedef my_stl::allocator<T>                        allocator_type;
        typedef my_stl::allocator<T>                        data_allocator;

        typedef typename allocator_type::value_type         value_type;
        typedef typename allocator_type::pointer            pointer;
        typedef typename allocator_type::const_pointer      const_pointer;
        typedef typename allocator_type::reference          reference;
        typedef typename allocator_type::const_reference    const_reference;
        typedef typename allocator_type::size_type          size_type;
        typedef typename allocator_type::difference_type    difference_type;

        typedef value_type*                                 iterator;
        typedef const value_type*                           const_iterator;
        typedef my_stl::reverse_iterator<iterator>          reverse_iterator;
        typedef my_stl::reverse_iterator<const_iterator>    const_reverse_iterator;

        allocator_type get_allocator() {return data_allocator();}

    private:
        /* 使用空间头部, 使用空间尾部, 存储空间尾部*/
        iterator begin_;
        iterator end_;
        iterator cap_;

    public:
        /* 构造，复制，移动，析构 */
        vector() noexcept {try_init();}

        /* 调用该类型的默认构造函数填充 */
        explicit vector(size_type n) {fill_init(n, value_type());}

        /* 指定值传入填充 */
        vector(size_type n, const value_type &value) {fill_init(n, value);}

        /* 范围构造 */
        template<class Iter, typename std::enable_if<
                my_stl::is_input_iterator<Iter>::value, int>::type = 0>
        vector(Iter first, Iter last) {
            MYSTL_DEBUG(!(last < first));
            range_init(first, last);
        }

        vector(const vector &rhs) {range_init(rhs.begin_, rhs.end_);}

        /* 移动构造只是指针赋值,然后清除原指针,并不用真正的move */
        vector(vector &&rhs) noexcept : begin_(rhs.begin_), end_(rhs.end_), cap_(rhs.cap_) {
            rhs.begin_ = nullptr;
            rhs.end_ = nullptr;
            rhs.cap_ = nullptr;
        }

        /* 初始化列表构造 */
        vector(std::initializer_list<value_type> list) {
            range_init(list.begin(), list.end());
        }

        /* 之后实现 */
        vector& operator=(const vector &rhs);
        vector& operator=(vector &&rhs) noexcept;
        vector& operator=(std::initializer_list<value_type> list) {
            vector temp(list.begin(), list.end());
            swap(temp);                                     /* 这个swap是类内的专属函数 */
            return *this;
        }

        /* 无情析构 */
        ~vector() {
            destroy_and_recover(begin_, end_, cap_ - begin_);
            begin_ = end_ = cap_ = nullptr;
        }

    public:
        /* 迭代器操作 */
        iterator begin()        noexcept {return begin_;}
        const_iterator begin()  const noexcept {return begin_;}
        iterator end()          noexcept {return end_;}
        const_iterator end()    const noexcept {return end_;}

        /* 反向迭代器 */
        reverse_iterator rbegin()           noexcept {return reverse_iterator(end_);}
        const_reverse_iterator  rbegin()    const noexcept {return reverse_iterator(end_);}
        reverse_iterator rend()             noexcept {return reverse_iterator(begin_);}
        const_reverse_iterator rend()       const noexcept {return reverse_iterator(begin_);}

        /* const迭代器 */
        const_iterator cbegin()             const noexcept {return begin_;}
        const_iterator cend()               const noexcept {return end_;}
        const_reverse_iterator crbegin()    const noexcept {return rbegin();}
        const_reverse_iterator crend()      const noexcept {return rend();}

        /* 容量操作 */
        bool empty() const noexcept {return begin_ == end_;}
        size_type size() const noexcept {return static_cast<size_type>(end_ - begin_);}
        size_type max_size() const noexcept {return static_cast<size_type>((-1) / sizeof(T));}
        size_type capacity() const noexcept {return static_cast<size_type>(cap_ - begin_);}

        /* 之后实现 */
        void reverse(size_type n);
        void shrink_to_fit();

        /* 元素访问操作 */
        reference operator[](size_type n) {
            MYSTL_DEBUG(n < size());
            return *(begin_ + n);
        }

        const_reference operator[](size_type n) const {
            MYSTL_DEBUG(n < size());
            return *(begin_ + n);
        }

        reference at(size_type n) {
            THROW_OUT_OF_RANGE_IF(!(n < size()), "vector<T>::at() subscript out of range.\n");
            return (*this)[n];
        }

        const_reference at(size_type n) const{
            THROW_OUT_OF_RANGE_IF(!(n < size()), "vector<T>::at() subscript out of range.\n");
            return (*this)[n];
        }

        reference front() {
            MYSTL_DEBUG(!empty());
            return *begin_;
        }

        const_reference front() const {
            MYSTL_DEBUG(!empty());
            return *begin_;
        }

        reference back() {
            MYSTL_DEBUG(!empty());
            return *(end_ - 1);
        }

        const_reference back() const {
            MYSTL_DEBUG(!empty());
            return *(end_ - 1);
        }

        /* 原来还有这个接口 */
        pointer data() noexcept {return begin_;}
        const_pointer data() const noexcept {return begin_;}

        /* 修改容器内部元素或结构操作 */
        /* assign */
        template<class Iter, typename std::enable_if<
                my_stl::is_input_iterator<Iter>::value, int>::type = 0>
        void assign(Iter first, Iter last) {
            MYSTL_DEBUG(!(last < first));
            copy_assign(first, last, iterator_category(first));
        }
        void assign(size_type n, const value_type &value) {fill_assign(n, value);}
        void assign(std::initializer_list<value_type> list) {copy_assign(list.begin(), list.end(), my_stl::forward_iterator_tag{});}

        /* emplace 和 emplace_back , 后面实现*/
        template<class ...Args>
        iterator emplace(const_iterator pos, Args &&... args);

        template<class ...Args>
        void emplace_back(Args &&... args);

        /* push_back 和 pop_back, 后面实现*/
        void push_back(const value_type &value);
        void push_back(value_type &&value) {emplace_back(my_stl::move(value));}
        void pop_back();

        /* 打印 */
        void print(const char *ends = " ");

        /* insert 后面实现*/
        template <class Iter, typename std::enable_if<
                my_stl::is_input_iterator<Iter>::value, int>::type = 0>
        void     insert(const_iterator pos, Iter first, Iter last) {
            MYSTL_DEBUG(pos >= begin() && pos <= end() && !(last < first));
            copy_insert(const_cast<iterator>(pos), first, last);
        }

        iterator insert(const_iterator pos, const value_type &value);
        iterator insert(const_iterator pos, value_type &&value) {return emplace(pos, my_stl::move(value));}
        iterator insert(const_iterator pos, size_type n, const value_type &value) {
            MYSTL_DEBUG(pos >= begin() && pos <= end());
            return fill_insert(const_cast<iterator>(pos), n, value);
        }

        /* erase 和 clear */
        iterator erase(const_iterator pos);
        iterator erase(const_iterator first, const_iterator last);
        void clear() {erase(begin(), end());}

        /* resize 和 reverse*/
        void resize(size_type new_size) {resize(new_size, size_type());}
        void resize(size_type new_size, const value_type &value);
        void reverse() { /* 独立实现的reverse，没有用算法库里面的reverse, 可能存在bug */
            auto p = begin();
            int left = 0, right = size() - 1;
            while (left < right) {
                my_stl::swap(*(p + left), *(p + right));
                left++;
                right--;
            }
        }

        /* swap 后面实现*/
        void swap(vector &rhs) noexcept;

    private:
        /* 辅助函数 */
        /* 初始化和析构*/
        void try_init() noexcept;
        void init_space(size_type size, size_type cap);
        void fill_init(size_type n, const value_type &value);
        template<class Iter> void range_init(Iter first, Iter last);
        void destroy_and_recover(iterator first, iterator last, size_type n);

        /* 计算扩展空间 */
        size_type get_new_cap(size_type add_size);

        /* 赋值 */
        void fill_assign(size_type n, const value_type &value);
        template<class Iter> void copy_assign(Iter first, Iter last, my_stl::input_iterator_tag);
        template<class Iter> void copy_assign(Iter first, Iter last, my_stl::forward_iterator_tag);

        /* 重新分配内存 */
        template<class ... Args>
        void reallocate_emplace(iterator pos, Args &&...args);
        void reallocate_insert(iterator pos, const value_type &value);

        /* 插入 */
        template<class Iter>
        void copy_insert(iterator pos, Iter first, Iter last);
        iterator fill_insert(iterator pos, size_type n, const value_type &value);

        /* shrink to fit */
        void reinsert(size_type size);
    };

    /**************************************************************************************
     * 一些运算符重载的具体实现
     **************************************************************************************/
     /* 赋值运算符 */
     template <class T>
     vector<T>& vector<T>::operator=(const vector &rhs) {
         if (this == &rhs)
             return *this;
         const auto len = rhs.size();
         /* 当赋值的大小大于当前容量，重新申请一个vector */
         if (len > capacity()) {
             vector tmp(rhs.begin(), rhs.end());
             swap(tmp);
         }
         /* 当前大小大于赋值的长度，复制然后删去多余的元素，再更新迭代器 */
         else if (size() >= len) {
             auto i = my_stl::copy(rhs.begin(), rhs.end(), begin());
             data_allocator::destroy(i, end_);
             end_ = begin() + len;
         }
         /* 当赋值的大小大于当前大小但小于当前容量,先赋值已经构造的，再初始化赋值后半段 */
         else {
             my_stl::copy(rhs.begin(), rhs.begin() + size(), begin_);
             my_stl::uninitialized_copy(rhs.begin() + size(), rhs.end(), end_);
             cap_ = end_ = begin_ + len;
         }
         return *this;
     }

     /* 移动赋值运算符 */
     template <class T>
     vector<T>& vector<T>::operator=(vector &&rhs) noexcept {
         destroy_and_recover(begin_, end_, cap_ - begin_);
         begin_ = rhs.begin_;
         end_= rhs.end_;
         cap_ = rhs.cap_;
         rhs.begin_ = nullptr;
         rhs.end_ = nullptr;
         rhs.cap_ = nullptr;
         return *this;
     }

     /* 若分配失败忽略，不抛出异常 */
     template <class T>
     void vector<T>::try_init() noexcept {
         try {
             begin_ = data_allocator::allocate(16);
             end_ = begin_;
             cap_ = begin_ + 16;
         } catch (...) {
             begin_ = nullptr;
             end_ = nullptr;
             cap_ = nullptr;
         }
     }

     /* 指定大小和存储空间,便于之后的空间扩展调用 */
     template <class T>
     void vector<T>::init_space(size_type size, size_type cap) {
         try {
             /* 分配内存 */
             begin_ = data_allocator::allocate(cap);
             end_ = begin_ + size;
             cap_ = begin_ + cap;
         } catch (...) {
             begin_ = nullptr;
             end_ = nullptr;
             cap_ = nullptr;
             throw ;
         }
     }

     /* fill_init */
     template <class T>
     void vector<T>::fill_init(size_type n, const value_type &value) {
         const size_type init_size = my_stl::max(static_cast<size_type>(16), n);
         init_space(n, init_size);
         my_stl::uninitialized_fill_n(begin_, n, value);
     }

     /* range_init */
     template <class T>
     template <class Iter>
     void vector<T>::range_init(Iter first, Iter last) {
         const size_type init_size = my_stl::max(static_cast<size_type>(last - first), static_cast<size_type>(16));
         init_space(static_cast<size_type>(last - first), init_size);
         my_stl::uninitialized_copy(first, last, begin_);
     }

     /* 析构，回收内存空间函数 */
     template <class T>
     void vector<T>::destroy_and_recover(iterator first, iterator last, size_type n) {
         data_allocator::destroy(first, last);
         data_allocator::deallocate(first, n);
     }

     /* 改变存储空间大小，当大于当前存储空间大小才会分配. 移动，更新迭代器*/
     template <class T>
     void vector<T>::reverse(size_type n) {
         if (capacity() < n) {
             THROW_LENGTH_ERROR_IF(n > max_size(),
                                   "can not larger than max_size() in vector<T>::reverse(n)");
             const auto old_size = size();
             auto temp = data_allocator::allocate(n);
             my_stl::uninitialized_move(begin_, end_, temp);
             begin_ = temp;
             end_ = temp + old_size;
             cap_ = begin_ + n;
         }
     }

     /* 放弃多余的容量 */
     template <class T>
     void vector<T>::shrink_to_fit() {
         if (end_ < cap_)
             reinsert(size());
     }

     /* 在pos位置原地构造元素，减少复制或者移动开销,这个函数有点迷惑 */
     template <class T>
     template <class ...Args>
     typename vector<T>::iterator vector<T>::emplace(const_iterator pos, Args &&...args) {
         MYSTL_DEBUG(pos >= begin() && pos <= end());
         iterator x_pos = const_cast<iterator> (pos);
         const size_type n = x_pos - begin_;
         if (end_ != cap_ && x_pos == end_) {
             data_allocator::construct(my_stl::address_of(*end_), my_stl::forward<Args>(args)...);
             ++end_;
         }
         else if (end_ != cap_) {
             auto new_end = end_;
             data_allocator::construct(my_stl::address_of(*end_), *(end_ - 1));
             ++new_end;
             my_stl::copy_backward(x_pos, end_ - 1, end_);
             *x_pos = value_type(my_stl::forward<Args>(args)...);
             //end_ = new_end;                                            //可能存在bug
         }
         else {
             reallocate_emplace(x_pos, my_stl::forward<Args>(args)...);
         }
         return begin() + n;
     }

     /* 尾部就地构造元素 */
     template <class T>
     template <class ...Args>
     void vector<T>::emplace_back(Args &&...args) {
         if (end_ < cap_) {
             data_allocator::construct(my_stl::address_of(*end_), my_stl::forward<Args>(args)...);
             ++end_;
         }
         else {
             reallocate_emplace(end_, my_stl::forward<Args>(args)...);
         }
     }

     /* 尾部插入元素 */
     template <class T>
     void vector<T>::push_back(const value_type &value) {
         if (end_ != cap_) {
             data_allocator::construct(my_stl::address_of(*end_), value);
             ++end_;
         }
         else {
             reallocate_insert(end_, value);
         }
     }

     /* 弹出尾部元素, 可能存在bug*/
     template <class T>
     void vector<T>::pop_back() {
         MYSTL_DEBUG(!empty());
         data_allocator::destroy(end_ - 1);
         --end_;
     }

    /* 在pos处插入元素 */
    template <class T>
    typename vector<T>::iterator vector<T>::insert(const_iterator pos, const value_type &value) {
        MYSTL_DEBUG(pos >= begin() && pos <= end());
        iterator x_pos = const_cast<iterator>(pos);
        const size_type n = pos - begin_;
        if (end_ != cap_ && x_pos == end_) {
            data_allocator::construct(my_stl::address_of(*end_), value);
            end_++;
        }
        else if (end_ != cap_) {
            auto new_end = end_;
            data_allocator::construct(my_stl::address_of(*end_), *(end_ - 1));
            ++new_end;
            auto value_copy = value;                        //避免元素因复制操作而被改变
            my_stl::copy_backward(x_pos, end_ - 1, end_);   //想象移动图
            *x_pos = my_stl::move(value_copy);
            end_ = new_end;
        }
        else {
            reallocate_insert(x_pos, value);
        }
        return begin_ + n;
    }

    /* 删除pos位置上的元素 */
    template <class T>
    typename vector<T>::iterator vector<T>::erase(const_iterator pos) {
        MYSTL_DEBUG(pos >= begin() && pos < end());
        iterator x_pos = begin_ + (pos - begin());
        my_stl::move(x_pos + 1, end_, x_pos);               //想象移动图
        data_allocator::destroy(end_ - 1);
        --end_;
        return x_pos;
    }

    /* 删除[first, last)上的元素*/
    template <class T>
    typename vector<T>::iterator
    vector<T>::erase(const_iterator first, const_iterator last) {
        MYSTL_DEBUG(first >= begin() && last <= end() && !(last < first));
        const auto n = first - begin();
        iterator r = begin_ + (first - begin());
        /* 把后面那一段移动到前面来, 再析构后面那一段 */
        data_allocator::destroy(my_stl::move(r + (last - first), end_, r), end_);
        end_ = end_ - (last - first);
        return begin_ + n;
    }

    /* 重置容器大小 */
    template <class T>
    void vector<T>::resize(size_type new_size, const value_type &value) {
        if (new_size < size())
            erase(begin() + new_size, end());
        else
            insert(end(), new_size - size(), value);
    }

    /* 得到新存储空间函数 */
    template <class T>
    typename vector<T>::size_type
    vector<T>::get_new_cap(size_type add_size) {
        /* 经验增加算法 */
        const auto old_size = capacity();
        THROW_LENGTH_ERROR_IF(old_size > max_size() - add_size, "vector<T>'s size too big..\n");
        if (old_size > max_size() - old_size / 2) {
            return old_size + add_size > max_size() - 16 ?
            old_size + add_size : old_size + add_size + 16;
        }
        const size_type new_size = old_size == 0 ?
                my_stl::max(add_size, static_cast<size_type>(16)) :
                my_stl::max(old_size + old_size / 2, old_size + add_size);
        return new_size;
    }

    /* 与另一个vector交换,只需交换指针 */
    template <class T>
    void vector<T>::swap(vector<T> &rhs) noexcept {
        if (this != &rhs) {
            my_stl::swap(begin_, rhs.begin_);
            my_stl::swap(end_, rhs.end_);
            my_stl::swap(cap_, rhs.cap_);
        }
    }

    template <class T>
    void vector<T>::fill_assign(size_type n, const value_type &value) {
        if (n > capacity()) {
            /* 直接申请新的 */
            vector temp(n, value);
            swap(temp);
        }
        else if (n > size()) {
            /* 先填充已经有的 */
            my_stl::fill(begin(), end(), value);
            /* 填充后段剩余的 */
            end_ = my_stl::uninitialized_fill_n(end_, n - size(), value);
        }
        else {
            erase(my_stl::fill_n(begin_, n, value), end_);
        }
    }

    template <class T>
    template <class Iter>
    void vector<T>::copy_assign(Iter first, Iter last, my_stl::input_iterator_tag) {
        auto cur = begin_;
        while (first != last && cur != end_) {
            *cur = *first;
            cur++;
            first++;
        }
        /* 缩小了 */
        if (first == last)
            erase(cur, end_);
        /* 变长了 */
        else
            insert(end_, first, last);
    }

    /* 用[first, last)给容器赋值 */
    template <class T>
    template <class Iter>
    void vector<T>::copy_assign(Iter first, Iter last, my_stl::forward_iterator_tag) {
        const size_type len = my_stl::distance(first ,last);
        if (len > capacity()) {
            vector temp(first, last);
            swap(temp);
        }
        else if (size() >= len) {
            /* 拷贝目标的范围元素 */
            auto new_end = my_stl::copy(first, last, begin_);
            /* 龙卷风摧毁原来多余的 */
            data_allocator::destroy(new_end, end_);
            end_ = new_end;
        }
        else {
            auto mid = first;
            /* 复制前半段 */
            my_stl::advance(mid, size());
            my_stl::copy(first, mid, begin_);
            /* 后半段再用构造进行赋值 */
            auto new_end = my_stl::uninitialized_copy(mid, last, end_);
            end_ = new_end;
        }
    }

    /* 重新分配空间且在pos处就地构造元素 */
    template <class T>
    template <class ...Args>
    void vector<T>::reallocate_emplace(iterator pos, Args &&...args) {
        /* 上面已经实现的经验空间分配算法计算新的大小 */
        const auto new_size = get_new_cap(1);
        auto new_begin = data_allocator::allocate(new_size);
        auto new_end = new_begin;
        try {
            /* 移动前半段 */
            new_end = my_stl::uninitialized_move(begin_, pos, new_begin);
            /* placement new 指定pos进行原地构造 */
            data_allocator::construct(my_stl::address_of(*new_end), my_stl::forward<Args>(args)...);
            /* 后半段再移动 */
            ++new_end;
            new_end = my_stl::uninitialized_move(pos, end_, new_end);
        } catch (...) {
            data_allocator::deallocate(new_begin, new_size);
            throw ;
        }
        /* 析构和释放原空间内容 */
        destroy_and_recover(begin_, end_, cap_ - begin_);
        begin_ = new_begin;
        end_ = new_end;
        cap_ = new_begin + new_size;
    }

    /* fill_insert */
    template <class T>
    typename vector<T>::iterator
    vector<T>::fill_insert(iterator pos, size_type n, const value_type &value) {
        if (n == 0)
            return pos;
        const size_type x_pos = pos - begin_;               /* begin到pos位置的距离*/
        const value_type value_copy = value;                /* 避免原值被覆盖 */
        if (static_cast<size_type>(cap_ - end_) >= n) {
            /* 剩余空间足够 */
            const size_type after_elements = end_ - pos;     /* pos到end的距离 */
            auto old_end = end_;
            if (after_elements > n) {
                my_stl::uninitialized_copy(end_ - n, end_, end_); /* 移动后半段 */
                end_ += n;                                        /* 更新end */
                my_stl::move_backward(pos, old_end - n, old_end); /* 填充位置的元素后移 */
                my_stl::uninitialized_fill_n(pos, n, value_copy); /* 填充新的值 */
            }
            else {
                /* 暴力填充 */
                end_ = my_stl::uninitialized_fill_n(end_, n - after_elements, value_copy);
                end_ = my_stl::uninitialized_move(pos, old_end, end_);
                my_stl::uninitialized_fill_n(pos, after_elements, value_copy);
            }
        }
        else {
            /* 剩余空间不足 */
            const auto new_size = get_new_cap(n);
            auto new_begin = data_allocator::allocate(new_size);
            auto new_end = new_begin;
            try {
                /* 分段处理 [begin, pos)  [pos, pos + n)  [pos + n, end) */
                new_end = my_stl::uninitialized_move(begin_, pos, new_begin);
                new_end = my_stl::uninitialized_fill_n(new_end, n, value_copy);
                new_end = my_stl::uninitialized_move(pos, end_, new_end);
            } catch (...) {
                /* 释放空间 */
                destroy_and_recover(new_begin, new_end, new_size);
                throw;
            }
            data_allocator::deallocate(begin_, cap_ - begin_);
            begin_ = new_begin;
            end_ = new_end;
            cap_ = new_begin + new_size;
        }
        return begin_ + x_pos;
    }

    /* reallocate_insert */
    template<class T>
    void vector<T>::reallocate_insert(iterator pos, const value_type &value) {
        const auto new_size = get_new_cap(1);
        auto new_begin = data_allocator::allocate(new_size);
        auto new_end = new_begin;
        const value_type &value_copy = value;
        try {
            /* 三段式扩展 */
            new_end = my_stl::uninitialized_move(begin_, pos, new_begin);
            data_allocator::construct(my_stl::address_of(*new_end), value_copy);
            ++new_end;
            new_end = my_stl::uninitialized_move(pos, end_, new_end);
        } catch (...) {
            data_allocator::deallocate(new_end, new_size);
            throw;
        }
        /* 析构和删除原有的东西 */
        destroy_and_recover(begin_, end_, cap_ - begin_);
        begin_ = new_begin;
        end_ = new_end;
        cap_ = new_begin + new_size;
    }

    /* copy_insert*/
    template <class T>
    template <class Iter>
    void vector<T>::copy_insert(iterator pos, Iter first, Iter last) {
        if (first == last)
            return;
        auto n = my_stl::distance(first, last);  /* 计算范围大小 */
        if (cap_ - end_ >= n){
            /* 剩余空间足够 */
            const auto after_elements = end_ - pos;
            auto old_end = end_;
            if (after_elements > n) {
                end_ = my_stl::uninitialized_copy(end_ - n, end_, end_); //构造后半段
                my_stl::move_backward(pos, old_end - n, old_end);        //移动原有的到后面
                my_stl::uninitialized_copy(first, last, pos);            //填充范围元素
            }
            else {
                auto mid = first;
                my_stl::advance(mid, after_elements);
                end_ = my_stl::uninitialized_copy(mid, last, end_);      //拷贝构造目标后半段
                end_ = my_stl::uninitialized_move(pos, old_end, end_);   //移动原有的
                my_stl::uninitialized_copy(first, mid, pos);             //拷贝构造目标前半段
            }
        }
        else {
            /* 剩余空间不足 */
            auto new_size = get_new_cap(n);
            auto new_begin = data_allocator::allocate(new_size);
            auto new_end = new_begin;
            try {
                /* 分段处理 [begin, pos)  [pos, pos + n)  [pos + n, end) */
                new_end = my_stl::uninitialized_move(begin_, pos, new_begin);
                new_end = my_stl::uninitialized_copy(first, last, new_end);
                new_end = my_stl::uninitialized_move(pos, end_, new_end);
            } catch (...) {
                destroy_and_recover(new_begin, new_end, new_size);
                throw;
            }
            data_allocator::deallocate(begin_, cap_ - begin_);
            begin_ = new_begin;
            end_ = new_end;
            cap_ = new_begin + new_size;
        }
    }

    template <class T>
    void vector<T>::reinsert(size_type size) {
        auto new_begin = data_allocator::allocate(size);
        try {
            my_stl::uninitialized_move(begin_, end_, new_begin);
        } catch (...) {
            data_allocator::deallocate(new_begin, size);
            throw;
        }
        data_allocator::deallocate(begin_, cap_ - begin_);
        begin_ = new_begin;
        end_ = begin_ + size;
        cap_ = begin_ + size;
    }

    template <class T>
    void vector<T>::print(const char *ends) {
        auto first = begin();
        auto last = end();
        while (first != last) {
            std::cout << *first << ends;
            first++;
        }
        std::cout << std::endl;
    }

    /**************************************************************************************
    * 重载比较运算符
    **************************************************************************************/
    /* 重载输出运算符 */
    template <class T>
    std::ostream& operator<<(std::ostream &os, const my_stl::vector<T> &vec) {
        for (auto it = vec.begin(); it != vec.end(); ++it) {
            os << *it << " ";
        }
        os << std::endl;
        return os;
    }

    template <class T>
    bool operator==(const vector<T> &lhs, const vector<T> &rhs) {
        return lhs.size() == rhs.size() && my_stl::equal(lhs.begin(), lhs.end(), rhs.begin());
    }

    template <class T>
    bool operator<(const vector<T> &lhs, const vector<T> &rhs) {
        return my_stl::s_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    template <class T>
    bool operator!=(const vector<T> &lhs, const vector<T> &rhs) {
        return !(rhs == lhs);
    }

    template <class T>
    bool operator>(const vector<T> &lhs, const vector<T> &rhs) {
        return rhs < lhs;
    }

    template <class T>
    bool operator<=(const vector<T> &lhs, const vector<T> &rhs) {
        return !(lhs > rhs);
    }

    template <class T>
    bool operator>=(const vector<T> &lhs, const vector<T> &rhs) {
        return !(lhs < rhs);
    }

    template <class T>
    void swap(vector<T> &lhs, vector<T> &rhs) {
        lhs.swap(rhs);
    }
}




#endif //MY_STL_MY_VECTOR_H
