add_llvm_component_library(LLVMHS32Disassembler
        HS32Disassembler.cpp

        LINK_COMPONENTS
        MCDisassembler
        HS32Info
        Support

        ADD_TO_COMPONENT
        HS32
        )