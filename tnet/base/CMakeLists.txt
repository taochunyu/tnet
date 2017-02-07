set(base_SRCS
  Exception.cc
  FileUtil.cc
  LogFile.cc
  Logging.cc
  LogStream.cc
  Thread.cc
  ThreadPool.cc
  TimerFd.cc
  Timestamp.cc
)

add_library(tnet_base ${base_SRCS})
set_target_properties(tnet_base PROPERTIES COMPILE_FLAGS "-std=c++0x")
