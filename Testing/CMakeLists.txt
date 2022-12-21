add_executable(Testing main.cpp integration_tests.cpp AbstractBitWriter_tests.cpp StreamingClusterer_tests.cpp Transformation_tests.cpp BlockReport_tests.cpp)
target_link_libraries(Testing Catch2::Catch2 AbstractBitWriter BitCounter VectorBitWriter Utilities BlockReport EvolutionaryFileCompressor)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pthread")

