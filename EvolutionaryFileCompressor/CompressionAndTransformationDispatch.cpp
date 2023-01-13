//
// Created by gian on 01/01/23.
//


#include "EvolutionaryFileCompressor.hpp"

#include "../Transformation/Transformations/DeltaTransform.hpp"
#include "../Transformation/Transformations/DeltaXORTransform.hpp"
#include "../Transformation/Transformations/RunLengthTransform.hpp"
#include "../Compression/HuffmanCompression/HuffmanCompression.hpp"
#include "../Compression/IdentityCompression/IdentityCompression.hpp"

#include "../Transformation/Transformations/SplitTransform.hpp"
#include "../Transformation/Transformations/SubtractAverageTransform.hpp"
#include "../Transformation/Transformations/SubtractXORAverageTransform.hpp"
#include "../Transformation/Transformations/StrideTransform.hpp"

#include "../Compression/NRLCompression/NRLCompression.hpp"
#include "../Transformation/Transformations/StackTransform.hpp"
#include "../Transformation/Transformations/IdentityTransform.hpp"
#include "../Compression/SmallValueCompression/SmallValueCompression.hpp"
#include "../Transformation/Transformations/LempelZivWelchTransform.hpp"
#include "../Compression/LZWCompression/LZWCompression.hpp"
#include "../Transformation/Transformations/BurrowsWheelerTransform.hpp"
#include "../Transformation/Transformations/SubMinAdaptiveTransform.hpp"

namespace GC {
    void EvolutionaryFileCompressor::applyCompressionCode(const EvolutionaryFileCompressor::CompressionCode &cc, const Block &block, AbstractBitWriter& writer) {
        switch (cc) {
            case C_IdentityCompression: return IdentityCompression().compress(block, writer);
            case C_HuffmanCompression: return HuffmanCompression().compress(block, writer);
            case C_RunLengthCompression: return NRLCompression().compress(block, writer);
            case C_SmallValueCompression: return SmallValueCompression().compress(block, writer);
            case C_LZWCompression: return LZWCompression().compress(block, writer);

        }
    }

    void EvolutionaryFileCompressor::applyTransformCode(const EvolutionaryFileCompressor::TransformCode &tc,
                                                        Block &block) {
#define GC_APPLY_T_CASE_X(TRANS, ...) case T_##TRANS : TRANS(__VA_ARGS__).apply(block);break
#define GC_APPLY_T_STRIDE_CASE_X(NUM) case T_StrideTransform_##NUM : StrideTransform(NUM).apply(block); break;
        switch (tc) {
            GC_APPLY_T_CASE_X(DeltaTransform);
            GC_APPLY_T_CASE_X(DeltaXORTransform);
            GC_APPLY_T_CASE_X(RunLengthTransform);
            GC_APPLY_T_CASE_X(StackTransform);
            GC_APPLY_T_CASE_X(SplitTransform);
            GC_APPLY_T_STRIDE_CASE_X(2);
            GC_APPLY_T_STRIDE_CASE_X(3);
            GC_APPLY_T_STRIDE_CASE_X(4);
            GC_APPLY_T_CASE_X(SubtractAverageTransform);
            GC_APPLY_T_CASE_X(SubtractXORAverageTransform);
            GC_APPLY_T_CASE_X(IdentityTransform);
            GC_APPLY_T_CASE_X(LempelZivWelchTransform);
            GC_APPLY_T_CASE_X(BurrowsWheelerTransform);
            GC_APPLY_T_CASE_X(SubMinAdaptiveTransform);
        }

    }

    void EvolutionaryFileCompressor::undoTransformCode(const TransformCode& tc, Block& block) {
        //LOG("undoing transform", Individual::TCode_as_string(tc), ", size was", block.size());

#define GC_UNDO_T_CASE(TRANS, ...) case T_##TRANS : TRANS(__VA_ARGS__).undo(block);break;
#define GC_UNDO_T_STRIDE_CASE(NUM) case T_StrideTransform_##NUM : StrideTransform(NUM).undo(block);break;
        switch (tc) {
            GC_UNDO_T_CASE(DeltaTransform);
            GC_UNDO_T_CASE(DeltaXORTransform);
            GC_UNDO_T_CASE(RunLengthTransform);
            GC_UNDO_T_CASE(SplitTransform);
            GC_UNDO_T_CASE(StackTransform);
            GC_UNDO_T_STRIDE_CASE(2);
            GC_UNDO_T_STRIDE_CASE(3);
            GC_UNDO_T_STRIDE_CASE(4);
            GC_UNDO_T_CASE(SubtractAverageTransform);
            GC_UNDO_T_CASE(SubtractXORAverageTransform);
            GC_UNDO_T_CASE(IdentityTransform);
            GC_UNDO_T_CASE(LempelZivWelchTransform);
            GC_UNDO_T_CASE(BurrowsWheelerTransform);
            GC_UNDO_T_CASE(SubMinAdaptiveTransform);
        }
        //LOG("The new block size is", block.size());
    }

    Block EvolutionaryFileCompressor::undoCompressionCode(const EvolutionaryFileCompressor::CompressionCode &cc, AbstractBitReader& reader) {
        switch (cc) {
            case C_IdentityCompression: return IdentityCompression().decompress(reader);
            case C_HuffmanCompression: return HuffmanCompression().decompress(reader);
            case C_RunLengthCompression: return NRLCompression().decompress(reader);
            case C_SmallValueCompression: return SmallValueCompression().decompress(reader);
            case C_LZWCompression: return LZWCompression().decompress(reader);
        }
    }
}