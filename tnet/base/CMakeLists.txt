set(TNET_BASE_SRCS
  Exception.cc
  FileUtil.cc
  LogFile.cc
  Logging.cc
  LogStream.cc
  Thread.cc
  ThreadPool.cc
  Timestamp.cc
)
message(STATUS "This is source dir " ${TNET_BASE_SRCS})
add_library(tnet_base ${TNET_BASE_SRCS})
target_link_libraries(tnet_base)
set_target_properties(tnet_base PROPERTIES COMPILE_FLAGS "-std=c++11")

install(TARGETS tnet_base DESTINATION lib)
