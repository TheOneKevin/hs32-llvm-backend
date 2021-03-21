add_llvm_component_library(LLVMHS32Desc
        HS32MCTargetDesc.cpp

        LINK_COMPONENTS
        MC
        MCDisassembler
        Support
        HS32Info

        ADD_TO_COMPONENT
        HS32
        )
