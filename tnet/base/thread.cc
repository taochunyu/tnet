#include <tnet/base/Thread.h>

namespace tnet {

Thread::Thread(const ThreadFunc &func, const std::string &name)
  : _started(false), _joined(false),  _func(func), _name(name)
{
}

}
