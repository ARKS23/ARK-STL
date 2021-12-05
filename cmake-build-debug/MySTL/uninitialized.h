//
// Created by 陈燊 on 2021/11/25.
//

#ifndef MY_STL_UNINITIALIZED_H
#define MY_STL_UNINITIALIZED_H

#include "algobase.h"
#include "construct.h"
#include "iterator.h"
#include "type_traits.h"
#include "util.h"
#include <iostream>

/*
 * 此头文件中的函数将用于对为初始化的空间进行元素构造
 */

namespace my_stl {
    /****************************************************************************************
     * uninitialized_copy
     * [first, last)上元素拷贝到result起始的空间,返回结束位置
     ****************************************************************************************/
     template <class InputIter, class ForwardIter>
     ForwardIter
     unchecked_uninitialized_copy(InputIter first, InputIter last, ForwardIter result, std::true_type) {
         return my_stl::copy(first, last, result);
     }

     /* 需要构造的型别版本 */
     template <class InputIter, class ForwardIter>
     ForwardIter
     unchecked_uninitialized_copy(InputIter first, InputIter last, ForwardIter result, std::false_type) {
         /* 副本迭代器，用于防止出现异常，可以进行回退 */
         auto cur = result;
         try {
             while (first != last) {
                 /* 定点构造,要取迭代器内容地址构造 */
                 my_stl::construct(&*cur, *first);
                 first++;
                 cur++;
             }
         } catch (...) {
             while (cur != result) {
                 my_stl::destroy(&*cur);
                 cur--;
             }
         }
         return cur;
     }

     template <class InputIter, class ForwardIter>
     ForwardIter
     uninitialized_copy(InputIter first, InputIter last, ForwardIter result) {
         /* 第四参数通过标准库的控件查看forwardIter里面的型别的赋值是不是无关痛痒的,激活型别推导 */
         return my_stl::unchecked_uninitialized_copy(first, last, result,
                                                     std::is_trivially_copy_assignable<
                                                     typename iterator_traits<ForwardIter>::
                                                     value_type>{});
     }

     /*****************************************************************************************
      * uninitialized_copy_n
      * 把[first, first + n)上内容复制到result为起始处的控件,返回复制结束的位置
      *****************************************************************************************/
      template <class InputIter, class Size, class ForwardIter>
      ForwardIter
      unchecked_uninitialized_copy_n(InputIter first, Size n, ForwardIter result, std::true_type) {
          return my_stl::copy_n(first, n, result).second;
      }

      template <class InputIter, class Size, class ForwardIter>
      ForwardIter
      unchecked_uninitialized_copy_n(InputIter first, Size n, ForwardIter result, std::false_type) {
          auto cur = result;
          try {
              while (n > 0) {
                  my_stl::construct(&*cur, *first);
                  --n;
                  ++cur;
                  ++first;
              }
          } catch (...) {
              while (result != cur) {
                  my_stl::destroy(&*cur);
                  --cur;
              }
          }
          return cur;
      }

      template <class InputIter, class Size, class ForwardIter>
      ForwardIter
      unchecked_uninitialized_copy_n(InputIter first, Size n, ForwardIter result) {
          return my_stl::unchecked_uninitialized_copy_n(first, n, result,
                                                        std::is_trivially_copy_assignable<
                                                        typename iterator_traits<InputIter>::
                                                        value_type>{});
      }

      /******************************************************************************************
       * uninitialized_fill
       * 在[first, last)区间内填充元素值
       ******************************************************************************************/
       template <class ForwardIter, class T>
       void
       unchecked_uninitialized_fill(ForwardIter first, ForwardIter last, const T &value, std::true_type) {
           my_stl::fill(first, last, value);
       }

       template <class ForwardIter, class T>
       void
       unchecked_uninitialized_fill(ForwardIter first, ForwardIter last, const T &value, std::false_type) {
           auto cur = first;
           try {
               while (cur != last) {
                   my_stl::construct(&*cur, value);
                   cur++;
               }
           } catch (...) {
               while (cur != first) {
                   my_stl::destroy(&*first);
                   ++first;
               }
           }
           return cur;
       }

       template <class ForwardIter, class T>
       void uninitialized_fill(ForwardIter first, ForwardIter last, const T &value) {
           my_stl::unchecked_uninitialized_fill(first, last, value,
                                                std::is_trivially_copy_assignable<
                                                typename iterator_traits<ForwardIter>
                                                ::value_type>{});
       }

       /*****************************************************************************************
        * fill_n
        * 从[first, first + n)填充元素
        *****************************************************************************************/
        template <class ForwardIter, class Size, class T>
        ForwardIter
        unchecked_un_init_fill_n(ForwardIter first, Size n, const T &value, std::true_type) {
            return my_stl::fill_n(first, n, value);
        }

        template <class ForwardIter, class Size, class T>
        ForwardIter
        unchecked_un_init_fill_n(ForwardIter first, Size n, const T &value, std::false_type) {
            auto cur = first;
            try {
                while (n > 0) {
                    my_stl::construct(&*cur, value);
                    cur++;
                }
            } catch (...) {
                while (first != cur) {
                    my_stl::destroy(&*first);
                    first++;
                }
            }
            return cur;
        }

        template <class ForwardIter, class Size, class T>
        ForwardIter
        uninitialized_fill_n(ForwardIter first, Size n, const T &value) {
            return unchecked_un_init_fill_n(first, n, value,
                                            std::is_trivially_copy_assignable<
                                            typename iterator_traits<ForwardIter>::
                                            value_type>{});
        }

       /******************************************************************************************
        * uninitialized_move
        * 把[first, last)上的内容移动到result为起始的控件,返回移动结束的位置
        ******************************************************************************************/
        template <class InputIter, class ForwardIter>
        ForwardIter
        unchecked_uninitialized_move(InputIter first, InputIter last, ForwardIter result, std::true_type) {
            return my_stl::move(first, last, result);
        }

        template <class InputIter, class ForwardIter>
        ForwardIter
        unchecked_uninitialized_move(InputIter first, InputIter last, ForwardIter result, std::false_type) {
            auto cur = result;
            try {
                while (cur != last) {
                    my_stl::construct(&*cur, my_stl::move(*first));
                    ++first;
                    ++cur;
                }
            } catch (...) {
                my_stl::destroy(result, cur);
            }
            return cur;
        }

        template <class InputIter, class ForwardIter>
        ForwardIter
        uninitialized_move(InputIter first, InputIter last, ForwardIter result) {
            return unchecked_uninitialized_move(first, last, result,
                                                std::is_trivially_copy_assignable<
                                                typename iterator_traits<InputIter>::
                                                value_type>{});
        }

        /******************************************************************************************
         * uninitialized_move_n
         * [first, first + n)移动至result为起始处的控件,返回移动结束的位置
         ******************************************************************************************/
         template <class InputIter, class Size, class ForwardIter>
         ForwardIter
         unchecked_uninitialized_move_n(InputIter first, Size n, ForwardIter result, std::true_type) {
             return my_stl::move(first, first + n, result);
         }

         template <class InputIter, class Size, class ForwardIter>
         ForwardIter
         unchecked_uninitialized_move_n(InputIter first, Size n, ForwardIter result, std::false_type) {
             auto cur = result;
             try {
                 while (n > 0) {
                     my_stl::construct(&*cur, my_stl::move(*first));
                     n--;
                 }
             } catch (...) {
                 while (result != cur) {
                     my_stl::destroy(&*result);
                     result++;
                 }
             }
             return cur;
         }

         template <class InputIter, class Size, class ForwardIter>
         ForwardIter
         uninitialized_move_n(InputIter first, Size n, ForwardIter result) {
             return my_stl::unchecked_uninitialized_move_n(first, n, result,
                                                           std::is_trivially_copy_assignable<
                                                           typename iterator_traits<InputIter>::
                                                           value_type>{});
         }
}

#endif //MY_STL_UNINITIALIZED_H
