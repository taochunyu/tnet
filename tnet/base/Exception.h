#ifndef TNET_BASE_EXCEPTION_H
#define TNET_BASE_EXCEPTION_H

#include <tnet/base/Types.h>
#include <string>
#include <exception>

namespace tnet {

class Exception : public std::exception {
 public:
  explicit Exception(const char*);
  explicit Exception(const std::string&);
  virtual ~Exception() throw();
  virtual const char* what() const throw();
  const char* stackTrace() const throw();
 private:
  void fillStackTrace();
  std::string _message;
  std::string _stack;
};

}

#endif  // TNET_BASE_EXCEPTION_H
