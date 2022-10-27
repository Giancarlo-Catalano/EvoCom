add_library(Evaluator Evaluator.cpp Evaluator.hpp)
add_subdirectory(BitCounter)
target_link_libraries(Evaluator BitCounter)

