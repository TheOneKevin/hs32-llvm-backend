add_llvm_component_library(LLVMHS32Desc
        HS32AsmBackend.cpp
        HS32ELFObjectWriter.cpp
        HS32MCAsmInfo.cpp
        HS32MCCodeEmitter.cpp
        HS32MCTargetDesc.cpp

        LINK_COMPONENTS
        MC
        MCDisassembler
        Support
        HS32Info

        ADD_TO_COMPONENT
        HS32
        )
