cmake_minimum_required(VERSION 3.23)
project(Diss_SimplePrototype)

set(CMAKE_CXX_STANDARD 20)


#add subdirectories here
add_subdirectory(Utilities)
add_subdirectory(BlockReport)
add_subdirectory(Transformation)
add_subdirectory(FileBitWriter)
add_subdirectory(FileBitReader)
add_subdirectory(StatisticalFeatures)
add_subdirectory(Compression)
add_subdirectory(HuffmanCoder)
add_subdirectory(SimpleCompressor)
add_subdirectory(DecisionTree)
add_subdirectory(Breeder)
add_subdirectory(Individual)
add_subdirectory(Random)
add_subdirectory(Selector)
add_subdirectory(Evolver)
add_subdirectory(PseudoFitness)

add_executable(Diss_SimplePrototype main.cpp Utilities names.hpp)

#declare which directories are used for linking
target_link_libraries(${PROJECT_NAME} Utilities Individual SimpleCompressor Breeder Random Selector Evolver PseudoFitness)






