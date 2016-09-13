set(base_SRCS
  Exception.cc
  FileUtil.cc
  LogFile.cc
  Logging.cc
  LogStream.cc
  Thread.cc
  ThreadPool.cc
  Timestamp.cc
)

add_library(tnet_base SHARED ${base_SRCS})
set_target_properties(tnet_base PROPERTIES COMPILE_FLAGS "-std=c++11")
