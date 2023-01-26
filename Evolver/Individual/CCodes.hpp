//
// Created by gian on 01/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_CCODES_HPP
#define DISS_SIMPLEPROTOTYPE_CCODES_HPP
#include "../../Utilities/utilities.hpp"

namespace GC {
    enum CCode {
        C_IdentityCompression,
        C_HuffmanCompression,
        C_RunLengthCompression,
        C_SmallValueCompression,
        C_LZWCompression
    };

    const std::vector<std::string> CCodesAsStrings = {
            "IDENT",
            "HUFFM",
            "RLCOM",
            "SMLVL",
            "LZWCM",
    };

    const std::vector<CCode> availableCCodes = {C_IdentityCompression, C_HuffmanCompression, C_RunLengthCompression, C_SmallValueCompression, C_LZWCompression};


    //TODO define a macro which does something for each compression method
    //it would be of the type FOR_EACH_COMP :: (CompEnumValue -> CompInstance -> Result) -> Result
    //and it would be implemented like this:
    /*
     * define FOR_EACH_COMP (COMP_MACRO)
     *          EVAL(COMP_MACRO(C_IdentityCompression, IndentityCompression())
     *          EVAL(COMP_MACRO(C_HuffmanCompression, HuffmanCompression())
     *          ...
     */


    }
#endif //DISS_SIMPLEPROTOTYPE_CCODES_HPP
