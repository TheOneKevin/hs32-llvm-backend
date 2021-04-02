add_llvm_component_library(LLVMHS32AsmParser
        HS32AsmParser.cpp

        LINK_COMPONENTS
        MC
        MCParser
        HS32Desc
        HS32Info
        Support

        ADD_TO_COMPONENT
        HS32
        )
