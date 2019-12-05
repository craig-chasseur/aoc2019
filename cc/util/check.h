#ifndef CC_UTIL_CHECK_H_
#define CC_UTIL_CHECK_H_

#include <cstdlib>
#include <iostream>

#include "absl/base/optimization.h"

#define CHECK(cond)                               \
  do {                                            \
    if (ABSL_PREDICT_FALSE(!(cond))) {            \
      std::cerr << "CHECK failure: " #cond "\n";  \
      std::exit(1);                               \
    }                                             \
  } while (false)

#endif  // CC_UTIL_CHECK_H_
