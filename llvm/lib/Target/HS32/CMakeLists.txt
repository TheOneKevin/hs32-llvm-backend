add_llvm_component_group(HS32)

add_llvm_target(HS32CodeGen
        HS32TargetMachine.cpp

        LINK_COMPONENTS
        Analysis
        AsmPrinter
        CodeGen
        Core
        MC
        SelectionDAG
        Support
        Target
        HS32Desc
        HS32Info

        ADD_TO_COMPONENT
        HS32
        )

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
