add_llvm_component_library(LLVMHS32Info
        HS32TargetInfo.cpp

        LINK_COMPONENTS
        Support

        ADD_TO_COMPONENT
        HS32
        )