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
        T_SubtractXORAverageTransform
    };

    const std::vector<std::string> TCodesAsStrings = {
            "T_IdentityTransform",
            "T_DeltaTransform",
            "T_DeltaXORTransform",
            "T_RunLengthTransform",
            "T_SplitTransform",
            "T_StackTransform",
            "T_StrideTransform_2",
            "T_StrideTransform_3",
            "T_StrideTransform_4",
            "T_SubtractAverageTransform",
            "T_SubtractXORAverageTransform"
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
                                                T_SubtractXORAverageTransform};


}

#endif //DISS_SIMPLEPROTOTYPE_TCODES_HPP
