#ifndef TNET_BASE_TYPES_H
#define TNET_BASE_TYPES_H

// implicit_cast would have been part of the C++ standard library,
// but the proposal was submitted too late.  It will probably make
// its way into the language in the future.
template<typename TO, typename FROM>
inline TO implicit_cast(const FROM &f) {
  return f;
}

// use like this: down_cast<T*>(foo);
// so we only accept pointers

template<typename TO, typename FROM>
inline TO down_cast(FROM *f) {
  // Ensures that To is a sub-type of From *.  This test is here only
  // for compile-time type checking, and has no overhead in an
  // optimized build at run-time, as it will be optimized away
  // completely.
  if (false) {
    implicit_cast<FROM*, TO>(0);
  }
  #if !defined(NDEBUG) && !defined(GOOGLE_PROTOBUF_NO_RTTI)
    assert(f == nullptr || dynamic_cast<TO>(f) != nullptr);  // RTTI: debug mode only!
  #endif
  return static_cast<TO>(f);
}

#endif  // TNET_BASE_TYPES_H
