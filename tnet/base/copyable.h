#ifndef TNET_BASE_COPYABLE_H
#define TNET_BASE_COPYABLE_H

namespace tnet {
  // A tag class emphasises the objects are copyable.
  // The empty base class optimization applies.
  // Any derived class of copyable should be a value type.
  class copyable {};
}

#endif  // TNET_BASE_COPYABLE_H
