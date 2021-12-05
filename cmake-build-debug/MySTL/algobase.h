//
// Created by 陈燊 on 2021/11/23.
//

#ifndef MY_STL_ALGOBASE_H
#define MY_STL_ALGOBASE_H
#include <cstring>
#include "iterator.h"
#include "util.h"

/*
 * 基本内部算法实现类（未测试）
 */

namespace my_stl {
#ifdef max
    #pragma message("#undefing marco max")
#undef max
#endif // max

#ifdef min
    #pragma message("#undefing marco min")
#undef min
#endif // min

    /* 定义max函数 */
    template <class T>
    const T& max(const T &lhs, const T &rhs) {
        return lhs < rhs ? rhs : lhs;
    }

    /* 用自定义比较函数获取大小 */
    template <class T, class Compare>
    const T& max(const T &lhs, const T &rhs, Compare comp) {
        return comp(lhs, rhs) ? rhs : lhs;
    }

    template <class T>
    const T& min(const T &lhs, const T &rhs) {
        return rhs < lhs ? rhs : lhs;
    }

    template <class T, class Compare>
    const T& min(const T &lhs, const T &rhs, Compare comp) {
        return comp(rhs, lhs) ? rhs : lhs;
    }

    /******************************************************************************************
     * iter_swap
     * 将迭代器所指之物对调
     ******************************************************************************************/
    template <class Iter1, class Iter2>
    void iter_swap(Iter1 lhs, Iter2 rhs) {
        my_stl::swap(*lhs, *rhs);
    }

    /******************************************************************************************
     * copy
     * [first, last)区间内的元素复制到[result, result + (last - first)) 之内
     ******************************************************************************************/
     //input_iterator版本
     template <class InputIter, class OutputIter>
     OutputIter
     unchecked_copy_cat(InputIter first, InputIter last, OutputIter result,
                        my_stl::input_iterator_tag) {
         while (first != last) {
             *result = *first;
             result++;
             first++;
         }
         return result;
     }

     //random_iterator版本
     template <class RandomIter, class OutputIter>
     OutputIter
     unchecked_copy_cat(RandomIter first, RandomIter last, OutputIter result,
                        my_stl::random_access_iterator_tag) {
         for (auto n = last - first; n > 0; --n, ++first, ++result) {
             *result = *first;
         }
         return result;
     }

     //上层调用
     template <class InputIter, class OutputIter>
     OutputIter
     unchecked_copy(InputIter first, InputIter last, OutputIter result) {
         /* 最后参数激活型别推导，调用对应的函数 */
         return unchecked_copy_cat(first, last, result, my_stl::iterator_category(first));
     }

    // 为 trivially_copy_assignable 类型提供特化版本,直接调用更底层的函数
    template <class Tp, class Up>
    typename std::enable_if<
            std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
            std::is_trivially_copy_assignable<Up>::value,
            Up*>::type
    unchecked_copy(Tp* first, Tp* last, Up* result) {
        const auto n = static_cast<size_t>(last - first);
        if (n != 0)
            std::memmove(result, first, n * sizeof(Up));
        return result + n;
    }

    /* 感觉调用存在二义性,感觉上面两个函数没有构成重载 */
    /* 疑惑解除: 有一个是针对原生指针的偏特化版本 */
    template <class InputIter, class OutputIter>
    OutputIter copy(InputIter first, InputIter last, OutputIter result) {
        return unchecked_copy(first, last, result);
    }

    /******************************************************************************************
     * copy_backward
     * 将[first, last)区间元素拷贝至[result - (last - first), result]内
     ******************************************************************************************/
    /* copy_backward_cat的双向迭代器版本 [first, last) -> [result - (last - first), result)*/
    template <class BidIter1, class BidIter2>
    BidIter2
    unchecked_copy_backward_cat(BidIter1 first, BidIter1 last, BidIter2 result,
                                my_stl::bidirectional_iterator_tag) {
        while (first != last) {
            *--result = *--last;
        }
        return result;
    }

    /* copy_backward_cat的随机迭代器版本 [first, last) -> [result - (last - first), result)*/
    template <class RandomIter1, class RandomIter2>
    RandomIter2
    unchecked_copy_backward_cat(RandomIter1 first, RandomIter1 last, RandomIter2 result,
                                my_stl::random_access_iterator_tag) {
        for (auto n = last - first; n > 0; --n)
            *--result = *--last;
        return result;
    }

    /* 上层调用 */
    template <class Iter1, class Iter2>
    Iter2
    unchecked_copy_backward(Iter1 first, Iter1 last, Iter2 result) {
        return unchecked_copy_backward_cat(first, last, result, my_stl::iterator_category(first));
    }

    // 为 trivially_copy_assignable 类型提供特化版本
    template <class Tp, class Up>
    typename std::enable_if<
            std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
            std::is_trivially_copy_assignable<Up>::value,
            Up*>::type
    unchecked_copy_backward(Tp* first, Tp* last, Up* result) {
        const auto n = static_cast<size_t>(last - first);
        if (n != 0) {
            result -= n;
            std::memmove(result, first, n * sizeof(Up));
        }
        return result;
    }

    template <class BidirectionalIter1, class BidirectionalIter2>
    BidirectionalIter2
    copy_backward(BidirectionalIter1 first, BidirectionalIter1 last, BidirectionalIter2 result) {
        return unchecked_copy_backward(first, last, result);
    }

    /*****************************************************************************************
     * copy_if
     * [first, last]内满足条件的拷贝到result为起点的区间上
     *****************************************************************************************/
     template <class InputIter, class OutputIter, class Judge>
     OutputIter
     copy_if(InputIter first, InputIter last, OutputIter result, Judge judge) {
        for (; first != last; ++first) {
            if (judge(*first))
                *result++ = *first;
        }
        return result;
     }

     /*****************************************************************************************
      * copy_n
      * 把[first, first + n]区间上的元素拷贝到[result, result + n)上
      * 返回一个pair指向拷贝结束的尾部
      *****************************************************************************************/
      template <class InputIter, class Size, class OutputIter>
      my_stl::pair<InputIter, OutputIter>
      unchecked_copy_n(InputIter first, Size n, OutputIter result, my_stl::input_iterator_tag) {
         for (; n > 0; --n, ++first, ++result)
             *result = *first;
         return my_stl::pair<InputIter, OutputIter>(first, result);
      }

      /* Random迭代器实现这个可以通过之前copy实现 */
      template <class RandomIter, class Size, class OutputIter>
      my_stl::pair<RandomIter, OutputIter>
      unchecked_copy_n(RandomIter first, Size n, OutputIter result, my_stl::random_access_iterator_tag) {
          auto last = first + n;
          return my_stl::pair<RandomIter, OutputIter>(last, my_stl::copy(first, last, result));
      }

      /* 上层调用 */
      template <class InputIter, class Size, class OutputIter>
      my_stl::pair<InputIter, OutputIter>
      copy_n(InputIter first, Size n, OutputIter result) {
          return unchecked_copy_n(first, n, result, my_stl::iterator_category(first));
      }

      /*****************************************************************************************
       * move
       * 把[first, last)区间元素移动到[result, result + (last - first))
       *****************************************************************************************/
       template <class InputIter, class OutputIter>
       OutputIter
       unchecked_move_cat(InputIter first, InputIter last, OutputIter result, my_stl::input_iterator_tag) {
          for (; first != last; ++first, ++result)
              *result = my_stl::move(*first);
          return result;
       }

       template <class RandomIter, class OutputIter>
       OutputIter
       unchecked_move_cat(RandomIter first, RandomIter last, OutputIter result, my_stl::random_access_iterator_tag) {
           for (auto n = last - first; n > 0; --n, ++first, ++result)
               *result = my_stl::move(*first);
           return result;
       }

       template <class InputIter, class OutputIter>
       OutputIter
       unchecked_move(InputIter first, InputIter last, OutputIter result) {
           return unchecked_move_cat(first, last, result, my_stl::iterator_category(first));
       }

        // 为 trivially_copy_assignable 类型提供特化版本,原生指针的偏特化版本
        template <class Tp, class Up>
        typename std::enable_if<
                std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
                std::is_trivially_move_assignable<Up>::value,
                Up*>::type
        unchecked_move(Tp* first, Tp* last, Up* result) {
            const size_t n = static_cast<size_t>(last - first);
            if (n != 0)
                std::memmove(result, first, n * sizeof(Up));
            return result + n;
        }

        /* 上层调用，下层自动判断原生指针和迭代器，迭代器又判断迭代器类型*/
        template <class InputIter, class OutputIter>
        OutputIter move(InputIter first, InputIter last, OutputIter result) {
            return unchecked_move(first, last, result);
        }

        /******************************************************************************************
         * move_backward
         * [first, last]区间内的元素移动到[result - (last - first), result)
         ******************************************************************************************/
         template <class BinaryIter1, class BinaryIter2>
         BinaryIter2
         unchecked_move_backward_cat(BinaryIter1 first, BinaryIter1 last,
                                     BinaryIter2 result, my_stl::bidirectional_iterator_tag) {
            while (first != last)
                *--result = my_stl::move(*--last);
            return result;
         }

         template <class RandomIter1, class RandomIter2>
         RandomIter2
         unchecked_move_backward_cat(RandomIter1 first, RandomIter1 last,
                                     RandomIter2 result, my_stl::random_access_iterator_tag) {
             for (auto n = last - first; n > 0; --n)
                 *--result = my_stl::move(*--last);
             return result;
         }

         template <class InputIter, class OutputIter>
         OutputIter
         unchecked_move_backward(InputIter first, InputIter last, OutputIter result) {
             return unchecked_move_backward_cat(first, last, result, my_stl::iterator_category(first));
         }

        // 为 trivially_copy_assignable 类型提供特化版本
        template <class Tp, class Up>
        typename std::enable_if<
                std::is_same<typename std::remove_const<Tp>::type, Up>::value &&
                std::is_trivially_move_assignable<Up>::value,
                Up*>::type
        unchecked_move_backward(Tp* first, Tp* last, Up* result) {
            const size_t n = static_cast<size_t>(last - first);
            if (n != 0) {
                result -= n;
                std::memmove(result, first, n * sizeof(Up));
            }
            return result;
        }

        template <class BidirectionalIter1, class BidirectionalIter2>
        BidirectionalIter2
        move_backward(BidirectionalIter1 first, BidirectionalIter1 last, BidirectionalIter2 result) {
            return unchecked_move_backward(first, last, result);
        }

        /******************************************************************************************
         * 比较区间内的元素是否都相等
         ******************************************************************************************/
         template <class InputIter1, class InputIter2>
         bool equal(InputIter1 first1, InputIter1 last1, InputIter2 first2) {
             for (; first1 != last1; ++first1, ++first2) {
                 if (*first1 != *first2)
                     return false;
             }
//            while (first1 != last1) {
//                if (*first1 != *first2)
//                    return false;
//                first1++;
//                first2++;
//            }
            return true;
         }

         /* 使用自定义compare */
         template <class InputIter1, class InputIter2, class Compare>
         bool equal(InputIter1 first1, InputIter1 last1, InputIter2 first2, Compare comp) {
             for (; first1 != last1; ++first1, ++first2) {
                 if (!comp(*first1, *first2))
                     return false;
             }
//             while (first1 != last1) {
//                 if (!comp(*first1, *first2))
//                     return false;
//                 first1++;
//                 first2++;
//             }
             return true;
         }

         /*******************************************************************************************
          * fill_n
          * 从first位置开始填充n个值
          *******************************************************************************************/
          /* 通用迭代器版本 */
          template <class OutputIter, class Size, class T>
          OutputIter unchecked_fill_n(OutputIter first, Size n, const T &value) {
             for (; n > 0; --n, ++ first)
                 *first = value;
             return first;
          }

          /* 针对原生指针提供的偏特化版本 */
          // 为 one-byte 类型提供特化版本

          template <class Tp, class Size, class Up>
          typename std::enable_if<
                    std::is_integral<Tp>::value && sizeof(Tp) == 1 &&
                    !std::is_same<Tp, bool>::value &&
                    std::is_integral<Up>::value && sizeof(Up) == 1,
                    Tp*>::type
          unchecked_fill_n(Tp* first, Size n, Up value) {
            if (n > 0) {
                std::memset(first, (unsigned char)value, (size_t)(n));
            }
            return first + n;
          }

          /* 上层调用版本 */
          template <class OutputIter, class Size, class T>
          OutputIter fill_n(OutputIter first, Size n, const T &value) {
              return unchecked_fill_n(first, n, value);
          }

          /********************************************************************************************
           * fill
           * [first, last)区间内填充新值
           ********************************************************************************************/
           /* 前向迭代器版本 */
           template <class ForwardIter, class T>
           void fill_cat(ForwardIter first, ForwardIter last, const T &value,
                                my_stl::forward_iterator_tag) {
                 for (; first != last; ++first)
                     *first = value;
//               while (first != last) {
//                   *first = value;
//                   first++;
//               }
           }

           /* 随机迭代器版本 */
           template <class RandomIter, class T>
           void fill_cat(RandomIter first, RandomIter last, const T &value,
                               my_stl::random_access_iterator_tag) {
               fill_n(first, last - first, value);
           }

           /* 上层调用版本, 激活型别推导 */
           template <class Iter, class T>
           void fill(Iter first, Iter last, const T &value) {
               fill_cat(first, last, value, my_stl::iterator_category(first));
           }

           /********************************************************************************************
            * s_compare
            * 字典序对两个序列进行比较
            * (1) 如果第一个序列元素较小返回true
            * (2) 如果到达last1但未到达last2返回true
            * (3) 如果未到达last1但到达last2返回false
            * (4) 同时到达返回false
            ********************************************************************************************/
            template <class InputIter1, class InputIter2>
            bool s_compare(InputIter1 first1, InputIter1 last1, InputIter2 first2, InputIter2 last2) {
                for (; first1 != last1 && first2 != last2; ++first1, ++first2) {
                    if (*first1 < *first2)
                        return true;
                    if (*first2 < *first1)
                        return false;
                }
//               while (first1 != last1 && first2 != last2) {
//                   if (*first1 < *first2)
//                       return true;
//                   else if (*first1 > *first2)
//                       return false;
//                   first1++;
//                   first2++;
//               }
               return first1 == last1 && first2 != last2;
            }

            /* 自定义比较器版本的 */
            template <class InputIter1, class InputIter2, class Compare>
            bool s_compare(InputIter1 first1, InputIter1 last1,
                           InputIter2 first2, InputIter2 last2, Compare comp) {
                for (; first1 != last1 && first2 != last2; ++first1, ++first2)
                {
                    if (comp(*first1, *first2))
                        return true;
                    if (comp(*first2, *first1))
                        return false;
                }
//                while (first1 != last1 && first2 != last2) {
//                    if (comp(*first1, *first2))
//                        return true;
//                    if (comp(*first1, *first2))
//                        return false;
//                    first1++;
//                    first2++;
//                }
                return first1 == last1 && first2 != last2;
            }

            // 针对 const unsigned char* 的特化版本
            bool s_compare(const unsigned char* first1,
                                         const unsigned char* last1,
                                         const unsigned char* first2,
                                         const unsigned char* last2) {
                const auto len1 = last1 - first1;
                const auto len2 = last2 - first2;
                // 先比较相同长度的部分
                const auto result = std::memcmp(first1, first2, my_stl::min(len1, len2));
                // 若相等，长度较长的比较大
                return result != 0 ? result < 0 : len1 < len2;
            }

            /******************************************************************************************
             * mismatch
             * 找到第一对失配的元素
             ******************************************************************************************/
             template <class InputIter1,class InputIter2>
             my_stl::pair<InputIter1, InputIter2>
             mismatch(InputIter1 first1, InputIter2 last1, InputIter2 first2) {
                while (first1 != last1 && *first1 == *first2) {
                    ++first1;
                    ++first2;
                }
                /* make_pair里面用到完美转发 */
                return my_stl::make_pair(first1, first2);
             }

             /* 自定义比较器版本 */
             template <class InputIter1, class InputIter2, class Compare>
             my_stl::pair<InputIter1, InputIter2>
             mismatch(InputIter1 first1, InputIter1 last1, InputIter2 first2) {
                 while (first1 != last1 && *first1 == *first2) {
                     ++first1;
                     ++first2;
                 }
                 /* make_pair里面完美转发 */
                 return make_pair(first1, first2);
             }
}


#endif //MY_STL_ALGOBASE_H
