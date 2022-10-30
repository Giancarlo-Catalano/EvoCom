//
// Created by gian on 01/10/22.
//

#ifndef DISS_SIMPLEPROTOTYPE_CCODES_HPP
#define DISS_SIMPLEPROTOTYPE_CCODES_HPP

namespace GC {
    enum CCode {
        C_IdentityCompression,
        C_HuffmanCompression,
        C_RunLengthCompression,
    };

    const std::vector<std::string> CCodesAsStrings = {
            "C_IdentityCompression",
            "C_HuffmanCompression",
            "C_RunLengthCompression"
    };

    const std::vector<CCode> availableCCodes = {C_HuffmanCompression,C_IdentityCompression,C_RunLengthCompression};

}
#endif //DISS_SIMPLEPROTOTYPE_CCODES_HPP
