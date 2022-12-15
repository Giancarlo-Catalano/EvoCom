//
// Created by gian on 01/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_TCODES_HPP
#define DISS_SIMPLEPROTOTYPE_TCODES_HPP

namespace GC {
    enum TCode {
        T_IdentityTransform,
        T_DeltaTransform,
        T_DeltaXORTransform,
        T_RunLengthTransform,
        T_SplitTransform,
        T_StackTransform,
        T_StrideTransform_2,
        T_StrideTransform_3,
        T_StrideTransform_4,
        T_SubtractAverageTransform,
        T_SubtractXORAverageTransform,
        T_LempelZivWelchTransform
    };

    const std::vector<std::string> TCodesAsStrings = {
            "IDENT",  //identity
            "DELTA",  //delta
            "D_XOR",  //delta xor
            "RLENC",   //run length encoding
            "SPLIT",  //byte split transform
            "STACK",  //Stack transform
            "STRD2",  //stride 2
            "STRD3",
            "STRD4",
            "SUBAV",  //subtract average transform
            "SBXAV",   //subtract xor average transform
            "LZWv5"     //lempel ziv welch version 5
    };

    const std::vector<TCode> availableTCodes = {T_IdentityTransform,
                                                T_DeltaTransform,
                                                T_DeltaXORTransform,
                                                T_RunLengthTransform,
                                                T_SplitTransform,
                                                T_StackTransform,
                                                T_StrideTransform_2,
                                                T_StrideTransform_3,
                                                T_StrideTransform_4,
                                                T_SubtractAverageTransform,
                                                T_SubtractXORAverageTransform,
                                                T_LempelZivWelchTransform};


}

#endif //DISS_SIMPLEPROTOTYPE_TCODES_HPP
