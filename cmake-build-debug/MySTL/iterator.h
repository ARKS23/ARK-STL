//
// Created by 陈燊 on 2021/11/20.
// 参考《STL源码剖析》, GitHub《MyTinySTL》
//

#ifndef MY_STL_ITERATOR_H
#define MY_STL_ITERATOR_H
#include "type_traits.h"

namespace my_stl{
    /* 五种迭代器 */
    struct input_iterator_tag {};   //只读迭代器
    struct output_iterator_tag {};  //只写迭代器
    struct forward_iterator_tag : public input_iterator_tag {};                 //前向迭代器
    struct bidirectional_iterator_tag : public forward_iterator_tag{};          //双向迭代器
    struct random_access_iterator_tag : public bidirectional_iterator_tag{};    //随机迭代器

    /* iterator 模板 */
    template <class Category, class T, class Distance = ptrdiff_t,
            class Pointer = T*, class Reference = T&>
    struct iterator {
        typedef Category    iterator_category;  //迭代器类型
        typedef T           value_type;         //迭代器对应型别
        typedef Pointer     pointer;            //型别对应指针
        typedef Reference   reference;          //型别对应引用
        typedef Distance    difference_type;    //指针距离型别
    };

    /* iterator traits , 这里迷惑 */
    template <class T>
    struct has_iterator_cat {
    private:
        struct two {char a; char b;};
        template<class U> static two test(...);
        template<class U> static char test(typename U::iterator_category* = 0);

    public:
        static const bool value = sizeof(test<T>(0)) == sizeof(char);
    };

    template <class Iterator, bool>
    struct iterator_traits_impl {};

    /* 萃取机 */
    template <class Iterator>
    struct iterator_traits_impl<Iterator, true> {
        typedef typename Iterator::iterator_category    iterator_category;
        typedef typename Iterator::value_type           value_type;;
        typedef typename Iterator::pointer              pointer;
        typedef typename Iterator::reference            reference;
        typedef typename Iterator:: difference_type     difference_type;
    };

    template <class Iterator, bool>
    struct iterator_traits_helper {};

    /* std::is_convertible用于判断是否可以转化过去*/
    /* 这里判断迭代器能不能转化成input 或者 output 迭代器*/
    template <class Iterator>
    struct iterator_traits_helper<Iterator, true>
            : public iterator_traits_impl<Iterator, std::is_convertible<typename Iterator::iterator_category, input_iterator_tag>::value ||
            std::is_convertible<typename Iterator::iterator_category, output_iterator_tag>::value> {};

    /* 萃取迭代器的特性 */
    template <class Iterator>
    struct iterator_traits : public iterator_traits_helper<Iterator, has_iterator_cat<Iterator>::value> {};

    /* 原生指针的偏特化版本 */
    template <class T>
    struct iterator_traits<T*> {
        typedef random_access_iterator_tag      iterator_category;
        typedef T                               value_type;
        typedef T*                              pointer;
        typedef T&                              reference;
        typedef ptrdiff_t                       difference_type;
    };

    template <class T>
    struct iterator_traits<const T*> {
        typedef random_access_iterator_tag      iterator_category;
        typedef T                               value_type;
        typedef const T*                        pointer;
        typedef const T&                        reference;
        typedef ptrdiff_t                       difference_type;
    };

    /* 这个不懂 */
    template <class T, class U, bool = has_iterator_cat<iterator_traits<T>>::value>
    struct has_iterator_cat_of
            : public m_bool_constant<std::is_convertible<
                    typename iterator_traits<T>::iterator_category, U>::value>
    {};

    /* 萃取某种迭代器 */
    template <class T, class U>
    struct has_iterator_cat_of<T, U, false> : public m_false_type {};

    template <class Iter>
    struct is_input_iterator : public has_iterator_cat_of<Iter, input_iterator_tag> {};

    template <class Iter>
    struct is_output_iterator : public has_iterator_cat_of<Iter, output_iterator_tag> {};

    template <class Iter>
    struct is_forward_iterator : public has_iterator_cat_of<Iter, forward_iterator_tag> {};

    template <class Iter>
    struct is_bidirectional_iterator : public has_iterator_cat_of<Iter, bidirectional_iterator_tag> {};

    template <class Iter>
    struct is_random_access_iterator : public has_iterator_cat_of<Iter, random_access_iterator_tag> {};

    template <class Iterator>
    struct is_iterator :
            public m_bool_constant<is_input_iterator<Iterator>::value ||
                                   is_output_iterator<Iterator>::value>
    {};

    /* 萃取某个迭代器的category */
    template <class Iterator>
    typename iterator_traits<Iterator>::iterator_category
    iterator_category(const Iterator&) {
        /* 从萃取机中获取迭代器类型 */
        typedef typename iterator_traits<Iterator>::iterator_category Category;
        return Category();
    }

    /* 萃取某个迭代器的 distance_type */
    template <class Iterator>
    typename iterator_traits<Iterator>::difference_type*
    distance_type(const Iterator&) {
        /* 野生指针强制转化成要萃取的指针类型 */
        return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
    }

    /* 萃取value_type */
    template <class Iterator>
    typename iterator_traits<Iterator>::value_type*
    value_type(const Iterator&) {
        return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
    }

    /* 计算迭代器之间的距离函数 */
    /* input_iterator_tag 的实现版本, 第三参数激活型别推导 */
    template <class InputIterator>
    typename iterator_traits<InputIterator>::difference_type
    distance_dispatch(InputIterator first, InputIterator last, input_iterator_tag) {
        typename iterator_traits<InputIterator>::difference_type n = 0;
        while (first != last) {
            ++first;
            ++n;
        }
        return n;
    }

    /* random_access_iterator 的实现版本*/
    template <class RandomIter>
    typename iterator_traits<RandomIter>::difference_type
    distance_dispatch(RandomIter first, RandomIter last, random_access_iterator_tag) {
        return last - first;
    }

    /* 上层通用版本 */
    template <class InputIterator>
    typename iterator_traits<InputIterator>::difference_type
    distance(InputIterator first, InputIterator last) {
        /* 调用上面的函数完成功能，第三个参数萃取型别构造临时类型激活实现函数的参数推导 */
        return distance_dispatch(first, last, iterator_category(first));
    }

    /* 让迭代器前进n个距离 */
    /* advance 的 input_iterator 版本, 第三参数激活型别推导*/
    template <class InputIterator, class Distance>
    void advance_dispatch(InputIterator &i, Distance n, input_iterator_tag) {
        while (n--)
            i++;
    }

    /* advance 的 bidi_iterator 版本*/
    template <class BidIter, class Distance>
    void advance_dispatch(BidIter &i, Distance n, bidirectional_iterator_tag) {
        if (n >= 0) while (n--) i++;
        else while (n++) i--;
    }

    /* advance 的 random_access_iterator版本 */
    template <class RandomIter, class Distance>
    void advance_dispatch(RandomIter &i, Distance n, random_access_iterator_tag) {
        i += n;
    }

    /* 上层通用版本 */
    template <class Iterator, class Distance>
    void advance(Iterator &i, Distance n) {
        /* 调用上面的函数完成功能，第三个参数萃取型别构造临时类型激活实现函数的参数推导 */
        advance_dispatch(i, n, iterator_category(i));
    }

    /************************************反向迭代器********************************************/
    template <class Iterator>
    class reverse_iterator {
    private:
        Iterator current;           //与此反向迭代器相对的正向迭代器
    public:
        /* 五种经典型别 */
        typedef typename iterator_traits<Iterator>::iterator_category iterator_category;
        typedef typename iterator_traits<Iterator>::value_type        value_type;
        typedef typename iterator_traits<Iterator>::difference_type   difference_type;
        typedef typename iterator_traits<Iterator>::pointer           pointer;
        typedef typename iterator_traits<Iterator>::reference         reference;

        typedef Iterator                                              iterator_type;
        typedef reverse_iterator<Iterator>                            self;

    public:
        /* 构造和拷贝构造, 要维护正确的正向迭代器，因为很多操作需要它来实现*/
        reverse_iterator() {}
        explicit reverse_iterator(iterator_type i) : current(i) {}
        reverse_iterator(const self& rhs) : current(rhs.current) {}

    public:
        /* 获得正向迭代器 */
        iterator_type base() const {
            return current;
        }

        /* 重载运算符 */
        reference operator*() const {
            /* 对应正向迭代器的前一个位置 */
            auto temp = current;
            return *--temp;
        }

        /* 里面用之前的实现 */
        pointer operator->() const {
            return &(operator*());
        }

        /* 与正向相反 */
        self& operator++() {
            --current;
            return *this;
        }

        self& operator--() {
            ++current;
            return *this;
        }

        self operator--(int) {
            self temp = *this;
            ++current;
            return temp;
        }

        self operator++(int) {
            self temp = *this;
            --current;
            return temp;
        }

        /* 自增运算符, 注意这里参数用对应的距离型别 */
        self& operator+=(difference_type n) {
            current -= n;
            return *this;
        }

        self& operator-=(difference_type n) {
            current += n;
            return *this;
        }

        /* 获得副本的加减运算符 */
        self operator+(difference_type n) const {
            return self(current - n);
        }

        self operator-(difference_type n) const {
            return self(current + n);
        }

        reference operator[](difference_type n) const {
            return *(*this + n);
        }
    };

    /* 重载-运算符 */
    template <class Iterator>
    typename reverse_iterator<Iterator>::difference_type
    operator-(const reverse_iterator<Iterator> &lhs, const reverse_iterator<Iterator> &rhs) {
        return lhs.base() - rhs.base();
    }

    /* 比较运算符 */
    template <class Iterator>
    bool operator==(const reverse_iterator<Iterator> &lhs, const reverse_iterator<Iterator> &rhs) {
        return lhs.base() == rhs.base();
    }

    template <class Iterator>
    bool operator!=(const reverse_iterator<Iterator> &lhs, const reverse_iterator<Iterator> &rhs) {
        return !(lhs == rhs);
    }

    template <class Iterator>
    bool operator<(const reverse_iterator<Iterator> &lhs, const reverse_iterator<Iterator> &rhs) {
        return lhs.base() < rhs.base();
    }

    template <class Iterator>
    bool operator>(const reverse_iterator<Iterator> &lhs, const reverse_iterator<Iterator> &rhs) {
        return lhs.base() > rhs.base();
    }

    template <class Iterator>
    bool operator>=(const reverse_iterator<Iterator> &lhs, const reverse_iterator<Iterator> &rhs) {
        return !(lhs.base() < rhs.base());
    }

    template <class Iterator>
    bool operator<=(const reverse_iterator<Iterator> &lhs, const reverse_iterator<Iterator> &rhs) {
        return !(lhs.base() > rhs.base());
    }
}


#endif //MY_STL_ITERATOR_H
