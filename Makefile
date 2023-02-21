
all: evocom
#CXXFLAGS := -std=c++20 -O3 -pthread
CXX := g++
CXXFLAGS := -std=c++17 -pthread


#General Utilities

utilities.o:
	$(CXX) -c $(CXXFLAGS) Utilities/utilities.cpp

StreamingClusterer.o:
	$(CXX) -c $(CXXFLAGS) Utilities/StreamingClusterer/StreamingClusterer.cpp

Logger.o:
	$(CXX) -c $(CXXFLAGS) Utilities/Logger/Logger.cpp

Writers := AbstractBitWriter.o FileBitWriter.o BitCounter.o

AbstractBitWriter.o:
	$(CXX) -c $(CXXFLAGS) AbstractBit/AbstractBitWriter/AbstractBitWriter.cpp

FileBitWriter.o:
	$(CXX) -c $(CXXFLAGS) AbstractBit/FileBitWriter/FileBitWriter.cpp

BitCounter.o:
	$(CXX) -c $(CXXFLAGS) Evolver/Evaluator/BitCounter/BitCounter.cpp


Readers := AbstractBitReader.o FileBitReader.o

AbstractBitReader.o:
	$(CXX) -c $(CXXFLAGS) AbstractBit/AbstractBitReader/AbstractBitReader.cpp

FileBitReader.o:
	$(CXX) -c $(CXXFLAGS) AbstractBit/FileBitReader/FileBitReader.cpp

sais.o:
	$(CXX) -c $(CXXFLAGS) Dependencies/SAIS/sais.c

LZW.o:
	$(CXX) -c $(CXXFLAGS) Dependencies/LZW/LZW.cpp

HuffmanCoder.o:
	$(CXX) -c $(CXXFLAGS) HuffmanCoder/HuffmanCoder.cpp

BlockReport.o: StatisticalFeatures.o
	$(CXX) -c $(CXXFLAGS) BlockReport/BlockReport.cpp

StatisticalFeatures.o:
	$(CXX) -c $(CXXFLAGS) StatisticalFeatures/StatisticalFeatures.cpp

RunningAverage.o:
	$(CXX) -c $(CXXFLAGS) StatisticalFeatures/RunningAverage.cpp



##Randoms

Randoms := RandomChance.o RandomElement.o RandomIndex.o RandomInt.o

RandomChance.o:
	$(CXX) -c $(CXXFLAGS) Random/RandomChance.cpp

RandomElement.o:
	$(CXX) -c $(CXXFLAGS) Random/RandomElement.cpp

RandomIndex.o:
	$(CXX) -c $(CXXFLAGS) Random/RandomIndex.cpp

RandomInt.o:
	$(CXX) -c $(CXXFLAGS) Random/RandomInt.cpp


##Evolver
Evolver.o: Recipe.o Breeder.o Selector.o Evaluator.o Logger.o RunningAverage.o
	$(CXX) -c $(CXXFLAGS) Evolver/Evolver.cpp

Recipe.o:
	$(CXX) -c $(CXXFLAGS) Evolver/Recipe/Recipe.cpp

Breeder.o: $(Randoms) Recipe.o
	$(CXX) -c $(CXXFLAGS) Evolver/Breeder/Breeder.cpp

Selector.o: $(Randoms) Recipe.o
	$(CXX) -c $(CXXFLAGS) Evolver/Selector/Selector.cpp

Evaluator.o: $(Randoms) PseudoFitness.o
	$(CXX) -c $(CXXFLAGS) Evolver/Evaluator/Evaluator.cpp

PseudoFitness.o: $(Randoms)
	$(CXX) -c $(CXXFLAGS) Evolver/PseudoFitness/PseudoFitness.cpp


##All the transformations


Transformation.o:
	$(CXX) -c $(CXXFLAGS) Transformation/Transformation.cpp

Transforms := BurrowsWheelerTransform.o DeltaTransform.o DeltaXORTransform.o IdentityTransform.o LempelZivWelchTransform.o RunLengthTransform.o SplitTransform.o StackTransform.o StrideTransform.o SubMinAdaptiveTransform.o SubtractAverageTransform.o SubtractXORAverageTransform.o

TRANSFORMS_DIR := Transformation/Transformations

BurrowsWheelerTransform.o: Transformation.o sais.o
	$(CXX) -c $(CXXFLAGS) $(TRANSFORMS_DIR)/BurrowsWheelerTransform.cpp

DeltaTransform.o: Transformation.o
	$(CXX) -c $(CXXFLAGS) $(TRANSFORMS_DIR)/DeltaTransform.cpp

DeltaXORTransform.o: Transformation.o
	$(CXX) -c $(CXXFLAGS) $(TRANSFORMS_DIR)/DeltaXORTransform.cpp

IdentityTransform.o: Transformation.o
	$(CXX) -c $(CXXFLAGS) $(TRANSFORMS_DIR)/IdentityTransform.cpp

LempelZivWelchTransform.o: Transformation.o LZW.o
	$(CXX) -c $(CXXFLAGS) $(TRANSFORMS_DIR)/LempelZivWelchTransform.cpp

RunLengthTransform.o: Transformation.o
	$(CXX) -c $(CXXFLAGS) $(TRANSFORMS_DIR)/RunLengthTransform.cpp

SplitTransform.o: Transformation.o
	$(CXX) -c $(CXXFLAGS) $(TRANSFORMS_DIR)/SplitTransform.cpp

StackTransform.o: Transformation.o
	$(CXX) -c $(CXXFLAGS) $(TRANSFORMS_DIR)/StackTransform.cpp

StrideTransform.o: Transformation.o
	$(CXX) -c $(CXXFLAGS) $(TRANSFORMS_DIR)/StrideTransform.cpp

SubMinAdaptiveTransform.o: Transformation.o
	$(CXX) -c $(CXXFLAGS) $(TRANSFORMS_DIR)/SubMinAdaptiveTransform.cpp

SubtractAverageTransform.o: Transformation.o
	$(CXX) -c $(CXXFLAGS) $(TRANSFORMS_DIR)/SubtractAverageTransform.cpp

SubtractXORAverageTransform.o: Transformation.o
	$(CXX) -c $(CXXFLAGS) $(TRANSFORMS_DIR)/SubtractXORAverageTransform.cpp

## Compressions

Compression.o:
	$(CXX) -c $(CXXFLAGS) Compression/Compression.cpp


COMPRESSION_DIR := Compression

Compressions := HuffmanCompression.o NRLCompression.o IdentityCompression.o LZWCompression.o SmallValueCompression.o

HuffmanCompression.o: Compression.o HuffmanCoder.o
	$(CXX) -c $(CXXFLAGS) $(COMPRESSION_DIR)/HuffmanCompression/HuffmanCompression.hpp


NRLCompression.o: Compression.o
	$(CXX) -c $(CXXFLAGS) $(COMPRESSION_DIR)/NRLCompression/NRLCompression.cpp

IdentityCompression.o: Compression.o
	$(CXX) -c $(CXXFLAGS) $(COMPRESSION_DIR)/IdentityCompression/IdentityCompression.cpp

LZWCompression.o: Compression.o LZW.o
	$(CXX) -c $(CXXFLAGS) $(COMPRESSION_DIR)/LZWCompression/LZWCompression.cpp

SmallValueCompression.o: Compression.o
	$(CXX) -c $(CXXFLAGS) $(COMPRESSION_DIR)/SmallValueCompression/SmallValueCompression.cpp



#EvoCompressor

CompressionAndTransformationDispatch.o: $(Transforms) $(Compressions)
	$(CXX) -c $(CXXFLAGS) EvolutionaryFileCompressor/CompressionAndTransformationDispatch.cpp

EvolutionaryFileCompressor.o: $(Readers) $(Writers) CompressionAndTransformationDispatch.o Evolver.o StreamingClusterer.o StatisticalFeatures.o
	$(CXX) -c $(CXXFLAGS) EvolutionaryFileCompressor/EvolutionaryFileCompressor.cpp




allObjects := AbstractBitReader.o AbstractBitWriter.o BitCounter.o LZW.o Breeder.o BurrowsWheelerTransform.o CompressionAndTransformationDispatch.o Compression.o DeltaTransform.o DeltaXORTransform.o Evaluator.o EvolutionaryFileCompressor.o Evolver.o FileBitReader.o FileBitWriter.o HuffmanCoder.o IdentityCompression.o IdentityTransform.o LempelZivWelchTransform.o Logger.o LZWCompression.o main.o NRLCompression.o PseudoFitness.o BlockReport.o RandomChance.o RandomElement.o RandomIndex.o RandomInt.o Recipe.o RunLengthTransform.o RunningAverage.o sais.o Selector.o SmallValueCompression.o SplitTransform.o StackTransform.o StatisticalFeatures.o StreamingClusterer.o StrideTransform.o SubMinAdaptiveTransform.o SubtractAverageTransform.o SubtractXORAverageTransform.o Transformation.o utilities.o

main.o: EvolutionaryFileCompressor.o utilities.o
	$(CXX) -c $(CXXFLAGS) main.cpp



evocom: main.o $(allObjects)
	$(CXX) $(CXXFLAGS) -o evocom $^

