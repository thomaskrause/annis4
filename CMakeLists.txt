project(graphANNIS)
cmake_minimum_required(VERSION 3.2)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(GLOBAL_OUTPUT_PATH ${PROJECT_BINARY_DIR}/output)

set(ENABLE_VALGRIND FALSE CACHE BOOL "Allow to turn Valgrind instrumentation on or off for some parts of the code")
set(ENABLE_SIMD_SUPPORT OFF CACHE BOOL "Enable support for operations using SIMD instructions. Needs a CPU with AVX2.")

# make sure a build type is selected
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(needed_features
  cxx_auto_type
  cxx_override
  cxx_nullptr
  cxx_long_long_type
  cxx_deleted_functions
  cxx_static_assert)

include(ExternalProject)
include(cmake/humblelogging.cmake)
include(cmake/re2.cmake)
include(cmake/celero.cmake)
include(cmake/googletest.cmake)
if(ENABLE_SIMD_SUPPORT)
  include(cmake/vc.cmake)
endif()

link_directories(${GLOBAL_OUTPUT_PATH})



IF(CMAKE_BUILD_TYPE MATCHES Debug)
  option(GRAPHANNIS_BUILD_BENCHMARKS "If ON build the benchmarks" OFF)
ELSE()
  option(GRAPHANNIS_BUILD_BENCHMARKS "If ON build the benchmarks" ON)
ENDIF()


find_package(ICU 44 REQUIRED)
set(Boost_USE_STATIC_LIBS   OFF)
find_package(Boost REQUIRED COMPONENTS chrono system filesystem thread)
find_package (Threads REQUIRED)

# write found packages to property file which is used by maven
file(WRITE "${GLOBAL_OUTPUT_PATH}/libdirs.properties" "Boost_INCLUDE_DIRS=${Boost_INCLUDE_DIRS}\n")
file(APPEND "${GLOBAL_OUTPUT_PATH}/libdirs.properties" "ICU_INCLUDE_DIRS=${ICU_INCLUDE_DIRS}\n")
file(APPEND "${GLOBAL_OUTPUT_PATH}/libdirs.properties" "Boost_LIBRARY_DIRS=${Boost_LIBRARY_DIRS}\n")



include_directories(
  "include"
)

include_directories(
  SYSTEM
  "ext/cpp-btree-1.0.1"
  ${HumbleLogging_INCLUDE_DIRS}
  ${ICU_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${RE2_INCLUDE_DIRS}
  "ext/cereal-1.2.1/include"
)


set(SRC_LIST_LIB
  src/lib/annis/queryconfig.cpp
  src/lib/annis/filter/binaryfilter.cpp
  src/lib/annis/json/jsoncpp.cpp
  src/lib/annis/json/jsonqueryparser.cpp
  src/lib/annis/db.cpp
  src/lib/annis/dbloader.cpp
  src/lib/annis/wrapper.cpp
  src/lib/annis/graphstorage/adjacencyliststorage.cpp
  src/lib/annis/graphstorage/prepostorderstorage.cpp
  src/lib/annis/graphstorage/linearstorage.cpp
  src/lib/annis/graphstorage/graphstorage.cpp
  src/lib/annis/dbcache.cpp
  src/lib/annis/stringstorage.cpp
  src/lib/annis/operators/operator.cpp
  src/lib/annis/operators/overlap.cpp
  src/lib/annis/operators/pointing.cpp
  src/lib/annis/operators/precedence.cpp
  src/lib/annis/operators/identicalcoverage.cpp
  src/lib/annis/operators/identicalnode.cpp
  src/lib/annis/operators/inclusion.cpp
  src/lib/annis/operators/dominance.cpp
  src/lib/annis/operators/abstractedgeoperator.cpp
  src/lib/annis/operators/partofsubcorpus.cpp
  src/lib/annis/api/corpusstoragemanager.cpp
  src/lib/annis/api/admin.cpp
  src/lib/annis/api/graph.cpp
  src/lib/annis/api/graphupdate.cpp
  src/lib/annis/query/singlealternativequery.cpp
  src/lib/annis/query/query.cpp
  src/lib/annis/util/dfs.cpp
  src/lib/annis/util/plan.cpp
  src/lib/annis/util/getRSS.cpp
  src/lib/annis/util/relannisloader.cpp
  src/lib/annis/util/sharedqueue.cpp
  src/lib/annis/util/threadpool.cpp
  src/lib/annis/annostorage.cpp
  src/lib/annis/graphstorageregistry.cpp
  src/lib/annis/join/donothingjoin.cpp
  src/lib/annis/join/indexjoin.cpp
  src/lib/annis/join/nestedloop.cpp
  src/lib/annis/join/threadnestedloop.cpp
  src/lib/annis/join/threadindexjoin.cpp
  src/lib/annis/annosearch/exactannokeysearch.cpp
  src/lib/annis/annosearch/exactannovaluesearch.cpp
  src/lib/annis/annosearch/regexannosearch.cpp
  src/lib/annis/annosearch/estimatedsearch.cpp
  src/lib/annis/annosearch/nodebyedgeannosearch.cpp
)

set(SRC_LIST_RUNNER
  src/runner/console.h
  src/runner/main.cpp
  src/runner/linenoise.h
  src/runner/linenoise.c
  src/runner/console.cpp
)

if(ENABLE_SIMD_SUPPORT)
  add_definitions(-DENABLE_SIMD_SUPPORT)
  set(SRC_LIST_LIB_SIMD src/lib/annis/join/simdindexjoin.cpp)
  if(MSVC)
    set_source_files_properties(${SRC_LIST_LIB_SIMD} PROPERTIES COMPILE_FLAGS "/arch:AVX2")
  else()
    set_source_files_properties(${SRC_LIST_LIB_SIMD} PROPERTIES COMPILE_FLAGS "-mavx2")
  endif()
  set_source_files_properties(${SRC_LIST_LIB_SIMD} PROPERTIES COMPILE_DEFINITIONS "Vc_IMPL=AVX2")
else()
  set(SRC_LIST_LIB_SIMD "")
endif()

if(ENABLE_VALGRIND)
  add_definitions(-DENABLE_VALGRIND)
endif()


include_directories("src/lib")

add_library(annis ${SRC_LIST_LIB} ${SRC_LIST_LIB_SIMD})
target_compile_features(annis PRIVATE ${needed_features})

set_property(TARGET annis PROPERTY POSITION_INDEPENDENT_CODE TRUE)

# newer Boost serialization versions are otherwise incompatible with BTree
add_definitions(-DBOOST_NO_CXX11_HDR_UNORDERED_MAP -DBOOST_NO_CXX11_HDR_UNORDERED_SET)
if (MSVC)
   # the min/max defines get in our way when using std::min/std::max
   add_definitions(/DNOMINMAX)
elseif ( CMAKE_COMPILER_IS_GNUCC )
   if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.1.0)
     set_property( TARGET annis APPEND_STRING PROPERTY COMPILE_FLAGS -Wsuggest-override )
   else()
     # workaround for some problems with older versions of GCC and Vc
     set_property( TARGET annis APPEND_STRING PROPERTY COMPILE_FLAGS -fabi-version=0 )
   endif()
endif ()

target_link_libraries(annis ${CMAKE_THREAD_LIBS_INIT} ${HumbleLogging_LIBRARIES} ${Boost_LIBRARIES} ${RE2_LIBRARIES} ${Vc_STATIC_LIBRARY} )

add_dependencies(annis HumbleLogging)
add_dependencies(annis RE2)	
if(ENABLE_SIMD_SUPPORT)
  add_dependencies(annis Vc)
endif()

add_executable(annis_runner ${SRC_LIST_RUNNER})
target_compile_features(annis_runner PRIVATE ${needed_features})
target_link_libraries(annis_runner annis)

# install the graphANNIS API and console
install(TARGETS annis_runner annis
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
       INCLUDES DESTINATION include
)
install(DIRECTORY src/lib/annis/api DESTINATION include/annis
        FILES_MATCHING PATTERN "*.h"
)
# also install the cereal headers since they are part of the API
install(DIRECTORY ext/cereal-1.2.1/include/ DESTINATION include
        FILES_MATCHING PATTERN "*.hpp"
)

################################
# Testing with Google Test
################################
enable_testing()
include_directories(SYSTEM ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

set(TEST_SRC_LIST
  src/tests/testlogger.h
  src/tests/SearchTestPcc2.h
  src/tests/SearchTestParlament.h
  src/tests/SearchTestRidges.h
  src/tests/CorpusStorageManagerTest.h
  src/tests/SearchTestGUM.h
  src/tests/LoadTest.h
  src/tests/SearchTestTiger.h
  src/tests/DFSTest.h
  src/tests/testmain.cpp
)

set(TEST_EXEC "test_${PROJECT_NAME}")

add_executable(${TEST_EXEC}  ${TEST_SRC_LIST} )
add_dependencies(${TEST_EXEC} annis GoogleTest)

target_compile_features(${TEST_EXEC} PRIVATE ${needed_features})
target_link_libraries(${TEST_EXEC} ${GoogleTest_LIBRARIES} annis)
add_test( ${TEST_EXEC} ${TEST_EXEC} )


if(GRAPHANNIS_BUILD_BENCHMARKS)

  # a benchmark using (rel)ANNIS as baseline
  set(ANNIS_BENCHMARK_SRC_LIST
    src/benchmarks/dynamicbenchmark.cpp
    src/benchmarks/benchmarkmain.cpp
  )
  set(ANNIS_BENCHMARK_EXEC "bench_annisqueries")
  add_executable(${ANNIS_BENCHMARK_EXEC}  ${ANNIS_BENCHMARK_SRC_LIST} )
  add_dependencies(${ANNIS_BENCHMARK_EXEC} annis Celero)
  target_compile_features(${ANNIS_BENCHMARK_EXEC} PRIVATE ${needed_features})
  target_link_libraries(${ANNIS_BENCHMARK_EXEC} ${CMAKE_THREAD_LIBS_INIT} ${Celero_LIBRARIES} annis  )


endif()
