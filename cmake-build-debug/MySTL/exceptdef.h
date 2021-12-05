//
// Created by 陈燊 on 2021/11/23.
//

#ifndef MY_STL_EXCEPTDEF_H
#define MY_STL_EXCEPTDEF_H

#include <stdexcept>
#include <cassert>

/* 参照MyTinySTL的头文件, 用于适定条件抛出异常 */
namespace my_stl {
#define MYSTL_DEBUG(expr) \
  assert(expr)

#define THROW_LENGTH_ERROR_IF(expr, what) \
  if ((expr)) throw std::length_error(what)

#define THROW_OUT_OF_RANGE_IF(expr, what) \
  if ((expr)) throw std::out_of_range(what)

#define THROW_RUNTIME_ERROR_IF(expr, what) \
  if ((expr)) throw std::runtime_error(what)

}


#endif //MY_STL_EXCEPTDEF_H
