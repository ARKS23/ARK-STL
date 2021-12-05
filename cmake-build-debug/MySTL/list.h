//
// Created by 陈燊 on 2021/12/3.
//

#ifndef MY_STL_LIST_H
#define MY_STL_LIST_H
#include <initializer_list>
#include "iterator.h"
#include "memory.h"
#include "functional.h"
#include "util.h"
#include "exceptdef.h"
#include <iostream>

/*
 * 模板类List
 * 双向链表
 *
 * 参考《MyTinySTL》
 * 异常保证:
 * 满足基本的异常保证，部分函数没有异常保证，对以下函数做强安全保证:
 *   emplace_front
 *   emplace_back
 *   emplace
 *   push_front
 *   push_back
 *   insert
 */

/*
 * line 768可能存在bug，待测试, 测试通过
 */

namespace my_stl {
    template <class T> struct list_node_base;
    template <class T> struct list_node;

    /* 萃取机 */
    template <class T>
    struct node_traits {
        typedef list_node_base<T>*  base_ptr;
        typedef list_node<T>*       node_ptr;
    };

    /*
     * list结点结构
     * 与LC上简易链表结构有许多不同。
     * 里面包含型别萃取，断开链接，获取自身指针的操作.
     */
    template <class T>
    struct list_node_base {
        typedef typename node_traits<T>::base_ptr base_ptr;
        typedef typename node_traits<T>::node_ptr node_ptr;

        base_ptr prev;      /* 前驱结点 */
        base_ptr next;      /* 后继结点 */

        list_node_base() = default;
        base_ptr self() {return static_cast<base_ptr>(&*this);}
        node_ptr as_node() {return static_cast<node_ptr>(self());}
        void unlink() {prev = next = self();}
    };

    /*
     * list_node
     * 其实就是list_node_base 加上数据域和拷贝构造和移动构造
     */
    template <class T>
    struct list_node : public list_node_base<T>{
        typedef typename node_traits<T>::base_ptr base_ptr;
        typedef typename node_traits<T>::node_ptr node_ptr;

        T value;                                                    /* 数据域 */

        list_node() = default;
        list_node(const T &v) : value(v) {}
        list_node(const T &&v) : value(my_stl::move(v)) {}
        base_ptr as_base() {return static_cast<base_ptr>(&*this);}  /* 这是继承关系，可以转 */
        node_ptr self() {return static_cast<node_ptr>(&*this);}
    };

    /*
     * list的迭代器设计
     * 它是双向迭代器，继承双向迭代器tag.
     */
    template <class T>
    struct list_iterator : public my_stl::iterator<my_stl::bidirectional_iterator_tag, T> {
        typedef T                                   value_type;
        typedef T*                                  pointer;
        typedef T&                                  reference;
        typedef typename node_traits<T>::base_ptr   base_ptr;
        typedef typename node_traits<T>::node_ptr   node_ptr;
        typedef list_iterator<T>                    self;

        base_ptr node_;                               /* 当前结点 */

        /* 构造函数 */
        list_iterator() = default;
        list_iterator(base_ptr ptr) : node_(ptr) {}
        list_iterator(node_ptr ptr) : node_(ptr->as_base()) {}
        list_iterator(const list_iterator &rhs) : node_(rhs.node_) {}

        /* 重载两种指针相关的操作赋 */
        reference operator*() const {return node_->as_node()->value;}
        pointer   operator->() const {return &(operator*());}

        /* 重载一元运算符 */
        self& operator++() {
            MYSTL_DEBUG(node_ != nullptr);
            node_ = node_->next;
            return *this;
        }

        self operator++(int) {
            MYSTL_DEBUG(node_ != nullptr);
            self temp = *this;
            ++*this;
            return temp;
        }

        self& operator--() {
            MYSTL_DEBUG(node_ != nullptr);
            node_ = node_->prev;
            return *this;
        }

        self operator--(int) {
            MYSTL_DEBUG(node_ != nullptr);
            self temp = *this;
            --*this;
            return temp;
        }

        /* 重载比较运算符 */
        bool operator==(const self &rhs) const {return node_ == rhs.node_;}
        bool operator!=(const self &rhs) const {return node_ != rhs.node_;}
    };

    /* 底层const迭代器设计 */
    template <class T>
    struct list_const_iterator : public iterator<bidirectional_iterator_tag, T> {
        typedef T                                   value_type;
        typedef const T*                            pointer;
        typedef const T&                            reference;
        typedef typename node_traits<T>::node_ptr   node_ptr;
        typedef typename node_traits<T>::base_ptr   base_ptr;
        typedef list_const_iterator<T>              self;

        base_ptr node_;

        list_const_iterator() = default;
        list_const_iterator(base_ptr ptr) : node_(ptr) {}
        list_const_iterator(node_ptr ptr) : node_(ptr->as_base()) {}
        list_const_iterator(const list_iterator<T> &rhs) : node_(rhs.node_) {}
        list_const_iterator(const list_const_iterator<T> &rhs) : node_(rhs.node_) {}

        reference operator*() const {return node_->as_node()->value;}
        pointer   operator->() const {return &(operator*());}

        self& operator++()
        {
            MYSTL_DEBUG(node_ != nullptr);
            node_ = node_->next;
            return *this;
        }
        self operator++(int)
        {
            self tmp = *this;
            ++*this;
            return tmp;
        }
        self& operator--()
        {
            MYSTL_DEBUG(node_ != nullptr);
            node_ = node_->prev;
            return *this;
        }
        self operator--(int)
        {
            self tmp = *this;
            --*this;
            return tmp;
        }

        // 重载比较操作符
        bool operator==(const self& rhs) const { return node_ == rhs.node_; }
        bool operator!=(const self& rhs) const { return node_ != rhs.node_; }
    };

    /* List */
    template <class T>
    class list {
    public:
        typedef my_stl::allocator<T>                    allocate_type;
        typedef my_stl::allocator<T>                    data_allocator;
        typedef my_stl::allocator<list_node_base<T>>    base_allocator;
        typedef my_stl::allocator<list_node<T>>         node_allocator;

        typedef typename allocate_type::value_type      value_type;
        typedef typename allocate_type::pointer         pointer;
        typedef typename allocate_type::const_pointer   const_pointer;
        typedef typename allocate_type::reference       reference;
        typedef typename allocate_type::const_reference const_reference;
        typedef typename allocate_type::size_type       size_type;
        typedef typename allocate_type::difference_type difference_type;

        typedef list_iterator<T>                         iterator;
        typedef list_const_iterator<T>                   const_iterator;
        typedef my_stl::reverse_iterator<iterator>       reverse_iterator;
        typedef my_stl::reverse_iterator<const_iterator> const_reverse_iterator;

        typedef typename node_traits<T>::base_ptr       base_ptr;
        typedef typename node_traits<T>::node_ptr       node_ptr;

        allocate_type get_allocator() {return node_allocator();}

    private:
        base_ptr node_;                                 /* 该指针指向末尾结点 */
        size_type size_;                                /* 链表大小 */

    public:
        list() {fill_init(0, value_type());}

        explicit list(size_type n) {fill_init(n, value_type());}

        list(size_type n, const T &value) {fill_init(n, value);}

        template <class Iter, typename std::enable_if<
                my_stl::is_input_iterator<Iter>::value, int>::type = 0>
        list(Iter first, Iter last)
        { copy_init(first, last); }

        list(std::initializer_list<T> i_list) {copy_init(i_list.begin(), i_list.end());}

        list(const list &rhs) {copy_init(rhs.begin(), rhs.end());}

        list(list &&rhs) noexcept : node_(rhs.node_), size_(rhs.size_) {
            rhs.node_ = nullptr;
            rhs.size_ = 0;
        }

        list& operator=(const list &rhs) {
            if (this != &rhs)
                assign(rhs.begin(), rhs.end());
            return *this;
        }

        list& operator=(list &&rhs) {
            clear();
            splice(end(), rhs);
            return *this;
        }

        list& operator=(std::initializer_list<T> i_list) {
            list temp(i_list.begin(), i_list.end());
            swap(temp);
            return *this;
        }

        ~list() {
            if (node_) {
                clear();
                base_allocator::deallocate(node_);
                node_ = nullptr;
                size_ = 0;
            }
        }

    public:
        /* 迭代器相关接口 */
        iterator begin() noexcept {return node_->next;}
        const_iterator begin() const noexcept {return node_->next;}
        iterator end() noexcept {return node_;}
        const_iterator end() const noexcept {return node_;}
        
        reverse_iterator rbegin() noexcept {return reverse_iterator(end());}
        const_reverse_iterator rbegin() const noexcept {return const_reverse_iterator(end());}
        reverse_iterator rend() noexcept {return reverse_iterator(begin());}
        const_reverse_iterator rend() const noexcept {return const_reverse_iterator(begin());}
        
        const_iterator cbegin() const noexcept {return begin();}
        const_iterator cend() const noexcept {return end();}
        const_reverse_iterator crbegin() const noexcept {return rbegin();}
        const_reverse_iterator crend() const noexcept {return rend();}
        
        /* 容器相关操作 */
        bool empty() {return node_->next == node_;}
        size_type size() {return size_;}
        size_type max_size() {return static_cast<size_type>(-1);}

        /* 访问元素相关操作 */
        reference front() {
            MYSTL_DEBUG(!empty());
            return *begin();
        }
        
        const_reference front() const {
            MYSTL_DEBUG(!empty());
            return *begin();
        }
        
        reference back() {
            MYSTL_DEBUG(!empty());
            return *(--end());
        }
        
        const_reference back() const {
            MYSTL_DEBUG(!empty());
            return *(--end());
        }

        /* 调整容器操作 */
        void assign(size_type n, const value_type &value) {fill_assign(n, value);}

        template <class Iter, typename std::enable_if<
                  my_stl::is_input_iterator<Iter>::value, int>::type = 0>
        void assign(Iter first, Iter last)
        { copy_assign(first, last); }

        void assign(std::initializer_list<T> i_list) {copy_assign(i_list.begin(), i_list.end());}

        /* emplace_front / emplace_back / emplace */
        template<class ...Args>
        void emplace_front(Args &&...args) {
            THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too long.\n");
            auto link_node = create_node(my_stl::forward<Args>(args)...);
            link_nodes_at_front(link_node->as_base(), link_node->as_base());
            ++size_;
        }

        template<class ...Args>
        void emplace_back(Args &&...args) {
            THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too long.\n");
            auto link_node = create_node(my_stl::forward<Args>(args)...);
            link_nodes_at_back(link_node->as_base(), link_node->as_base());
            ++size_;
        }

        template<class ...Args>
        iterator emplace(const_iterator pos, Args &&...args) {
            THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too long.\n");
            auto link_node = create_node(my_stl::forward<Args>(args)...);
            link_nodes(pos.node_, link_node->as_base(), link_node->as_base());
            ++size_;
            return iterator(link_node);
        }

        /* insert */
        iterator insert(const_iterator pos, const value_type &value) {
            THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
            auto link_node = create_node(value);
            ++size_;
            return link_iter_node(pos, link_node->as_base());
        }

        iterator insert(const_iterator pos, value_type &&value) {
            THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
            auto link_node = create_node(my_stl::move(value));
            ++size_;
            return link_iter_node(pos, link_node->as_base());
        }

        iterator insert(const_iterator pos, size_type n, const value_type &value) {
            THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
            return fill_insert(pos, n, value);
        }

        template <class Iter, typename std::enable_if<
                my_stl::is_input_iterator<Iter>::value, int>::type = 0>
        iterator insert(const_iterator pos, Iter first, Iter last)
        {
            size_type n = my_stl::distance(first, last);
            THROW_LENGTH_ERROR_IF(size_ > max_size() - n, "list<T>'s size too big");
            return copy_insert(pos, n, first);
        }

        /* push_back / push_front */
        void push_front(const value_type &value) {
            THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
            auto link_node = create_node(value);
            link_nodes_at_front(link_node->as_base(), link_node->as_base());
            ++size_;
        }

        void push_front(value_type &&value) {
            emplace_front(my_stl::move(value));
        }

        void push_back(const value_type &value) {
            THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
            auto link_node = create_node(value);
            link_nodes_at_back(link_node->as_base(), link_node->as_base());
            size_++;
        }

        void push_back(value_type &&value) {
            emplace_back(my_stl::move(value));
        }

        /* pop_back / pop_front */
        void pop_front() {
            MYSTL_DEBUG(!empty());
            auto node = node_->next;        /* 第一个结点就是node_的下一个 */
            unlink_nodes(node, node);
            destroy_node(node->as_node());
            --size_;
        }

        void pop_back() {
            MYSTL_DEBUG(!empty());
            auto node = node_->prev;       /* 最后一个结点就是node_的前一个 */
            unlink_nodes(node, node);
            destroy_node(node->as_node());
            --size_;
        }

        /* erase / clear */
        iterator erase(const_iterator pos);
        iterator erase(const_iterator first, const_iterator last);
        void clear();

        /* resize */
        void resize(size_type new_size) {resize(new_size, value_type());}
        void resize(size_type new_size, const value_type &value);
        void swap(list &rhs) noexcept {
            my_stl::swap(node_, rhs.node_);
            my_stl::swap(size_, rhs.size_);
        }

        /* list 核心操作 */
        void splice(const_iterator pos, list &other);
        void splice(const_iterator pos, list &other, const_iterator it);
        void splice(const_iterator pos, list &other, const_iterator first, const_iterator last);

        /* 相关操作 */
        template<class Unary>
        void remove_if(Unary pred);
        void remove(const value_type &value) {remove_if([&](const value_type &v) {return v == value;});}

        template<class Binary>
        void unique(Binary pred);
        void unique() {unique(my_stl::equal_to<T>());}

        template<class Compare>
        void merge(list &x, Compare comp);
        void merge(list &x) {merge(x, my_stl::less<T>());}

        template<class Compare>
        void sort(Compare comp) {list_sort(begin(), end(), size(), comp);}
        void sort() {list_sort(begin(), end(), size(), my_stl::less<T>());}

        void reverse();

    private:
        /* ************************************辅助函数***********************************************/
        /* 构造和析构结点 */
        template<class  ...Args>
        node_ptr create_node(Args &&...args);
        void destroy_node(node_ptr p);

        /* 初始化 */
        template<class Iter>
        void copy_init(Iter first, Iter last);
        void fill_init(size_type n, const value_type &value);

        /* 链接和断开链接操作 */
        iterator link_iter_node(const_iterator pos, base_ptr node);
        void link_nodes(base_ptr p, base_ptr first, base_ptr last);
        void link_nodes_at_front(base_ptr first, base_ptr last);
        void link_nodes_at_back(base_ptr first, base_ptr last);
        void unlink_nodes(base_ptr first, base_ptr last);

        /* 赋值 */
        template<class Iter>
        void copy_assign(Iter first, Iter last);
        void fill_assign(size_type n, const value_type &value);

        /* 插入 */
        template<class Iter>
        iterator copy_insert(const_iterator pos, size_type n, Iter first);
        iterator fill_insert(const_iterator pos, size_type n, const value_type &value);

        /* 排序 */
        template<class Compared>
        iterator list_sort(iterator first, iterator last, size_type n, Compared comp);
    };

    /* *************************************实现**************************************** */

    // 删除pos处元素
    template <class T>
    typename list<T>::iterator
    list<T>::erase(const_iterator pos) {
        MYSTL_DEBUG(pos != cend());
        auto n = pos.node_;
        auto next = n->next;        //用于之后构造一个迭代器使用
        unlink_nodes(n, n);
        destroy_node(n->as_node());
        --size_;
        return iterator(next);
    }

    // 删除[first, last)
    template <class T>
    typename list<T>::iterator
    list<T>::erase(const_iterator first, const_iterator last) {
        if (first != last) {
            unlink_nodes(first.node_, last.node_->prev);
            while (first != last) {
                auto cur = first.node_;
                ++first;
                destroy_node(cur->as_node());
                --size_;
            }
        }
        return iterator(last.node_);
    }

    // 清空list
    template <class T>
    void list<T>::clear() {
        if (size_ != 0) {
            auto cur = node_->next;
            for (base_ptr next = cur->next; cur != node_; cur = next, next = cur->next) {
                destroy_node(cur->as_node());
            }
            node_->unlink();
            size_ = 0;
        }
    }

    //重置链表大小
    template<class T>
    void list<T>::resize(size_type new_size, const value_type &value) {
        auto i = begin();
        size_type len = 0;
        while (i != end() && len < new_size) {
            ++i;
            ++len;
        }
        /* 截去多余的 */
        if (len == new_size)
            erase(i, node_);
        else {
            insert(node_, new_size - len, value);
        }
    }

    //将list other 接到pos之前
    template<class T>
    void list<T>::splice(const_iterator pos, list<T> &other) {
        MYSTL_DEBUG(this != &other);
        if (!other.empty()) {
            THROW_LENGTH_ERROR_IF(size_ > max_size() - other.size(), "list<T>'s size too big");
            auto first = node_->next;
            auto last = node_->prev;

            other.unlink_nodes(first, last);
            link_nodes(pos.node_, first, last);

            size_ += other.size_;
            other.size_ = 0;
        }
    }

    // 将it所指的结点结合到pos之前
    template<class T>
    void list<T>::splice(const_iterator pos, list<T> &other, const_iterator it) {
        if (pos.node_ != it.node_ && pos.node_ != it.node_->next) {
            THROW_LENGTH_ERROR_IF(size_ > max_size() - 1, "list<T>'s size too big");
            auto first = it.node_;
            other.unlink_nodes(first, first);
            link_nodes(pos.node_, first, first);
            ++size_;
            --other.size_;
        }
    }

    // 将list other的[first, last)元素接到pos之前
    template <class T>
    void list<T>::splice(const_iterator pos, list<T> &other, const_iterator first, const_iterator last) {
        if (first != last && this != &other) {
            size_type n = my_stl::distance(first, last);
            THROW_LENGTH_ERROR_IF(size_ > max_size() - n, "list<T>'s size too big");
            auto f = first.node_;
            auto l = last.node_;
            other.unlink_nodes(f, l);
            link_nodes(pos.node_, f, l);
            size_ += n;
            other.size_ -= n;
        }
    }

    // 移除符合预期的元素
    template <class T>
    template <class Unary>
    void list<T>::remove_if(Unary pred) {
        auto first = begin();
        auto last = end();
        for (auto next = first; first != last; first = next) {
            ++next;
            if (pred(*first))
                erase(first);
        }
    }

    //移除list中满足pred为true的元素
    template <class T>
    template <class Binary>
    void list<T>::unique(Binary pred) {
        auto i = begin();
        auto e = end();
        auto j = i;
        ++j;
        while (j != e) {
            if (pred(*i, *j))
                erase(j);
            else
                i = j;
            j = i;
            ++j;
        }
    }

    //与另一个list合并，按照comp排序
    template <class T>
    template <class Compare>
    void list<T>::merge(list<T> &x, Compare comp) {
        if (this != &x) {
            THROW_LENGTH_ERROR_IF(size_ > max_size() - x.size_, "list<T>'s size too big");
            auto first1 = begin();
            auto last1 = end();
            auto first2 = x.begin();
            auto last2 = x.end();

            while (first1 != last1 && first2 != last2) {
                if (comp(*first2, *first1)) {
                    auto next = first2;
                    ++next;
                    for (; next != last2 && comp(*next, *first1); ++next) {
                        auto f = first2.node_;
                        auto l = next.node_->prev;
                        first2 = next;

                        x.unlink_nodes(f, l);
                        link_nodes(first1.node_, f, l);
                        ++first1;
                    }
                }
                else {
                    ++first1;
                }
            }
            if (first2 != last2) {
                auto f = first2.node_;
                auto l = last2.node_;
                x.unlink_nodes(f, l);
                link_nodes(last1.node_, f, l);
            }

            size_ += x.size_;
            x.size_ = 0;
        }
    }

    // 反转
    template <class T>
    void list<T>::reverse() {
        if (size_ <= 1)
            return;
        auto i = begin();
        auto j = end();
        while (i.node_ != j.node_) {
            my_stl::swap(i.node_->prev, i.node_->next);
            i.node_ = i.node_->prev;
        }
        my_stl::swap(j.node_->prev, j.node_->next);
    }

    /* 辅助函数实现 */
    // 创建结点
    template <class T>
    template <class ...Args>
    typename list<T>::node_ptr
    list<T>::create_node(Args &&...args) {
        node_ptr p = node_allocator::allocate(1);
        try {
            data_allocator::construct(my_stl::address_of(p->value), my_stl::forward<Args>(args)...);
            p->next = nullptr;
            p->prev = nullptr;
        } catch (...) {
            node_allocator::deallocate(p);
            throw;
        }
        return p;
    }

    // 龙卷风摧毁结点
    template <class T>
    void list<T>::destroy_node(node_ptr p) {
        data_allocator::destroy(my_stl::address_of(p->value));
        node_allocator::deallocate(p);
    }

    // 用n个元素初始化容器
    template <class T>
    void list<T>::fill_init(size_type n, const value_type &value) {
        node_ = base_allocator::allocate(1);
        node_->unlink();
        size_ = n;
        try {
            for (; n > 0; --n) {
                auto node = create_node(value);
                link_nodes_at_back(node->as_base(), node->as_base());
            }
        } catch (...) {
            clear();
            base_allocator::deallocate(node_);
            node_ = nullptr;
            throw ;
        }
    }

    // [first, last)初始化
    template <class T>
    template <class Iter>
    void list<T>::copy_init(Iter first, Iter last) {
        node_ = base_allocator::allocate(1);
        node_->unlink();
        size_type n = my_stl::distance(first, last);
        size_ = n;
        try {
            for (; n > 0; --n, ++first) {
                auto node = create_node(*first);
                link_nodes_at_back(node->as_base(), node->as_base());
            }
        } catch (...) {
            clear();
            base_allocator::deallocate(node_);
            node_ = nullptr;
            throw;
        }
    }

    //在pos处连接一个结点
    template <class T>
    typename list<T>::iterator
    list<T>::link_iter_node(const_iterator pos, base_ptr node) {
        if (pos == node_->next) {
            link_nodes_at_front(node, node);
        }
        else if (pos == node_) {
            link_nodes_at_back(node, node);
        }
        else {
            link_nodes(pos.node_, node, node);
        }
        return iterator(node);
    }

    //pos处连接[first, last]
    template <class T>
    void list<T>::link_nodes(base_ptr pos, base_ptr first, base_ptr last) {
        pos->prev->next = first;
        first->prev = pos->prev;
        pos->prev = last;
        last->next = pos;
    }

    //头插,可能存在bug,测试通过, 无bug
    template <class T>
    void list<T>::link_nodes_at_front(base_ptr first, base_ptr last) {
        node_->next->prev = last;
        last->next = node_->next;
        node_->next = first;
        first->prev = node_;
    }

    //尾插 可能存在bug
    template <class T>
    void list<T>::link_nodes_at_back(base_ptr first, base_ptr last) {
        node_->prev->next = first;
        first->prev = node_->prev;
        last->next = node_;
        node_->prev = last;
    }

    // 容器与[first, last]结点断开连接
    template <class T>
    void list<T>::unlink_nodes(base_ptr first, base_ptr last) {
        first->prev->next = last->next;
        last->next->prev = first->prev;
    }

    // 用n个元素为容器赋值
    template <class T>
    void list<T>::fill_assign(size_type n, const value_type &value) {
        auto i = begin();
        auto e = end();
        for (; n > 0 && i != e; --n, ++i)
            *i = value;
        if (n > 0)
            insert(e, n, value);
        else
            erase(i, e);
    }

    // [first, last)赋值
    template <class T>
    template <class Iter>
    void list<T>::copy_assign(Iter first, Iter last) {
        auto f1 = begin();
        auto l1 = end();
        for (; f1 != l1 && first != last; ++f1, ++first)
            *f1 = first;
        if (last == first)
            erase(f1, l1);
        else
            insert(l1, first, last);
    }

    // pos处插入n个元素
    template <class T>
    typename list<T>::iterator
    list<T>::fill_insert(const_iterator pos, size_type n, const value_type &value) {
        iterator r(pos.node_);
        if (n != 0) {
            const auto add_size = n;
            auto node = create_node(value);
            r = iterator(node);
            iterator end = r;
            try {
                for (--n; n > 0; --n, ++end) {
                    auto next = create_node(value);
                    end.node_->next = next->as_base();
                    next->prev = end.node_;
                }
                size_ += add_size;
            } catch (...) {
                auto end_node = end.node_;
                while (true) {
                    auto prev = end_node->prev;
                    destroy_node(end_node->as_node());
                    if (prev == nullptr)
                        break;
                    end_node = prev;
                }
                throw ;
            }
            link_nodes(pos.node_, r.node_, end.node_);
        }
        return r;
    }

    //pos处插入[first, last)
    template <class T>
    template <class Iter>
    typename list<T>::iterator
    list<T>::copy_insert(const_iterator pos, size_type n, Iter first) {
        iterator r(pos.node_);
        if (n != 0) {
            const auto add_size = n;
            auto node = create_node(*first);
            node->prev = nullptr;
            r = iterator(node);
            iterator end = r;
            try {
                for (--n, ++first; n > 0; --n, ++first, ++end) {
                    auto next = create_node(*first);
                    end.node_->next = next->as_base();
                    next->prev = end.node_;
                }
                size_ += add_size;
            } catch (...) {
                while (true) {
                    auto end_node = end.node_;
                    auto prev = end_node->prev;
                    destroy_node(end_node->as_node());
                    if (prev == nullptr)
                        break;
                    end_node = prev;
                }
                throw ;
            }
            link_nodes(pos.node_, r.node_, end.node_);
        }
        return r;
    }

    //对list进行归并排序, 参考《MyTinySTL》
    template <class T>
    template <class Compared>
    typename list<T>::iterator
    list<T>::list_sort(iterator f1, iterator l2, size_type n, Compared comp) {
        if (n < 2)
            return f1;
        if (n == 2) {
            if (comp(*--l2, *f1)) {
                auto ln = l2.node_;
                unlink_nodes(ln, ln);
                link_nodes(f1.node_, ln, ln);
                return l2;
            }
            return f1;
        }

        auto n2 = n / 2;
        auto l1 = f1;
        my_stl::advance(l1, n2);
        auto result = f1 = list_sort(f1, l1, n2, comp);
        auto f2 = l1 = list_sort(l1, l2, n - n2, comp);

        if (comp(*f2, *f1)) {
            auto m = f2;
            ++m;
            for (; m != l2 && comp(*m, *f1); ++m)
                ;
            auto f = f2.node_;
            auto l = m.node_->prev;
            result = f2;
            l1 = f2 = m;
            unlink_nodes(f, l);
            m = f1;
            ++m;
            link_nodes(f1.node_, f, l);
            f1 = m;
        }
        else {
            ++f1;
        }

        // 合并两段有序区间
        while (f1 != l1 && f2 != l2)
        {
            if (comp(*f2, *f1))
            {
                auto m = f2;
                ++m;
                for (; m != l2 && comp(*m, *f1); ++m)
                    ;
                auto f = f2.node_;
                auto l = m.node_->prev;
                if (l1 == f2)
                    l1 = m;
                f2 = m;
                unlink_nodes(f, l);
                m = f1;
                ++m;
                link_nodes(f1.node_, f, l);
                f1 = m;
            }
            else
            {
                ++f1;
            }
        }
        return result;
    }

    // 重载比较操作符
    template <class T>
    bool operator==(const list<T>& lhs, const list<T>& rhs)
    {
        auto f1 = lhs.cbegin();
        auto f2 = rhs.cbegin();
        auto l1 = lhs.cend();
        auto l2 = rhs.cend();
        for (; f1 != l1 && f2 != l2 && *f1 == *f2; ++f1, ++f2)
            ;
        return f1 == l1 && f2 == l2;
    }

    template <class T>
    bool operator<(const list<T>& lhs, const list<T>& rhs)
    {
        return my_stl::s_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
    }

    template <class T>
    bool operator!=(const list<T>& lhs, const list<T>& rhs)
    {
        return !(lhs == rhs);
    }

    template <class T>
    bool operator>(const list<T>& lhs, const list<T>& rhs)
    {
        return rhs < lhs;
    }

    template <class T>
    bool operator<=(const list<T>& lhs, const list<T>& rhs)
    {
        return !(rhs < lhs);
    }

    template <class T>
    bool operator>=(const list<T>& lhs, const list<T>& rhs)
    {
        return !(lhs < rhs);
    }

// 重载 my_stl 的 swap
    template <class T>
    void swap(list<T>& lhs, list<T>& rhs) noexcept
    {
        lhs.swap(rhs);
    }

    //增加输出重载
    template <class T>
    std::ostream& operator<<(std::ostream &os, const list<T> &l) {
        for (auto it = l.begin(); it != l.end(); ++it) {
            os << *it << " ";
        }
        os << std::endl;
        return os;
    }
}

#endif //MY_STL_LIST_H
