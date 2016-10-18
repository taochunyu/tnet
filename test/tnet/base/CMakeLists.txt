# ExceptionTest
add_executable(ExceptionTest Exception_gtest.cc)
target_link_libraries(ExceptionTest tnet_base ${GTEST_LIBRARIES} ${GTEST_MAIN})
add_test(ExceptionTest ExceptionTest)

# FileUtilTest
add_executable(FileUtilTest FileUtil_gtest.cc)
target_link_libraries(FileUtilTest tnet_base ${GTEST_LIBRARIES} ${GTEST_MAIN})
add_test(FileUtilTest FileUtilTest)

# LoggingTest
add_executable(LoggingTest Logging_gtest.cc)
target_link_libraries(LoggingTest tnet_base ${GTEST_LIBRARIES} ${GTEST_MAIN})
add_test(LoggingTest LoggingTest)

# LogStreamTest
add_executable(LogStreamTest LogStream_gtest.cc)
target_link_libraries(LogStreamTest tnet_base ${GTEST_LIBRARIES} ${GTEST_MAIN})
add_test(LogStreamTest LogStreamTest)

# MutexTest
add_executable(MutexTest Mutex_gtest.cc)
target_link_libraries(MutexTest tnet_base ${GTEST_LIBRARIES} ${GTEST_MAIN})
add_test(MutexTest MutexTest)

# StringPieceTest
add_executable(StringPieceTest StringPiece_gtest.cc)
target_link_libraries(StringPieceTest tnet_base ${GTEST_LIBRARIES} ${GTEST_MAIN})
add_test(StringPieceTest StringPieceTest)

# ThreadPoolTest
add_executable(ThreadPoolTest ThreadPool_gtest.cc)
target_link_libraries(ThreadPoolTest tnet_base ${GTEST_LIBRARIES} ${GTEST_MAIN})
add_test(ThreadPoolTest ThreadPoolTest)

# TimestampTest
add_executable(TimestampTest Timestamp_gtest.cc)
target_link_libraries(TimestampTest tnet_base ${GTEST_LIBRARIES} ${GTEST_MAIN})
add_test(TimestampTest TimestampTest)

# ThreadTest
add_executable(ThreadTest Thread_gtest.cc)
target_link_libraries(ThreadTest tnet_base ${GTEST_LIBRARIES} ${GTEST_MAIN})
add_test(ThreadTest ThreadTest)
