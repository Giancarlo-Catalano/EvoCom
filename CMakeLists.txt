cmake_minimum_required(VERSION 3.23)
project(EvoCom)

set(CMAKE_CXX_STANDARD 20)




#add subdirectories here
add_subdirectory(Dependencies)

add_subdirectory(Utilities)
add_subdirectory(BlockReport)
add_subdirectory(Transformation)
add_subdirectory(StatisticalFeatures)
add_subdirectory(Compression)
add_subdirectory(HuffmanCoder)
add_subdirectory(EvolutionaryFileCompressor)
add_subdirectory(DecisionTree)

add_subdirectory(Random)
add_subdirectory(Evolver)
add_subdirectory(AbstractBit)



add_subdirectory(Testing)
enable_testing()


add_executable(EvoCom main.cpp Utilities names.hpp)

#declare which directories are used for linking
target_link_libraries(${PROJECT_NAME} Utilities Individual EvolutionaryFileCompressor Breeder Random Selector Evolver PseudoFitness Evaluator)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pthread")



