#ifndef CYGNI_ASSERT_HPP
#define CYGNI_ASSERT_HPP

#include <cassert>

#ifndef NDEBUG
#define CYGNI_ASSERT(expr, msg) assert((expr) && (msg))
#else
#define CYGNI_ASSERT(expr, msg) ((void)0)
#endif

#endif /* CYGNI_ASSERT_HPP */