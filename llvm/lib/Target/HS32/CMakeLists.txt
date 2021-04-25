add_llvm_component_group(HS32)

set(LLVM_TARGET_DEFINITIONS HS32.td)

tablegen(LLVM HS32GenAsmMatcher.inc         -gen-asm-matcher)
tablegen(LLVM HS32GenAsmWriter.inc          -gen-asm-writer)
tablegen(LLVM HS32GenRegisterInfo.inc       -gen-register-info)
tablegen(LLVM HS32GenInstrInfo.inc          -gen-instr-info)
tablegen(LLVM HS32GenSubtargetInfo.inc      -gen-subtarget)
tablegen(LLVM HS32GenMCCodeEmitter.inc      -gen-emitter)

add_public_tablegen_target(HS32CommonTableGen)

add_llvm_target(HS32CodeGen
        HS32Subtarget.cpp
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

add_subdirectory(AsmParser)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
