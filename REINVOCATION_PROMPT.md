# QP-QK SDK Project Reinvocation Prompt

## Project Overview
You are working on the QP-QK SDK project, creating modern tooling for the QP (Quantum Platform) embedded systems framework. The main focus is developing a web-based visual state machine editor as a VS Code extension.

## Current Status (As of 2025-01-24)

### Phase 1 ✅ COMPLETE (Tagged: phase1-complete)
- Created QP Mermaid VS Code extension
- Basic state machine visualization and code generation working
- GitHub.dev compatible
- Successfully tested with simple state machines

### Phase 2 🚧 IN PROGRESS
Major features implemented:
- ✅ Mermaid preprocessor for QP-specific syntax
- ✅ Hierarchical state machine support
- ✅ Enhanced code generator for nested states
- ✅ Semantic validation with VS Code diagnostics
- ✅ Complex examples (calculator, toaster oven)

Remaining Phase 2 tasks:
- [ ] IntelliSense support for QP keywords
- [ ] Unit tests
- [ ] Custom webview editor with drag-and-drop
- [ ] QM file import/export

## Project Structure
```
/home/murr2k/projects/qp/
├── QP-QK-SDK/
│   ├── tools/vscode-extensions/qp-mermaid/  # Main extension code
│   │   ├── src/                             # TypeScript source
│   │   ├── examples/                        # Example .qp.mmd files
│   │   └── package.json                     # Extension manifest
│   ├── docs/                                # Planning documents
│   └── examples/mermaid/                    # Example diagrams
├── PHASE1_RESTART_PROMPT.md                 # Original task
└── REINVOCATION_PROMPT.md                   # This file
```

## Key Technical Details

### Extension Features
1. **File Format**: `.qp.mmd` files (QP-enhanced Mermaid)
2. **Commands**: Preview (Ctrl+Shift+V), Generate Code, Validate
3. **Parser**: Handles QP syntax (entry/exit actions, guards, hierarchical states)
4. **Generator**: Produces QP/C code with proper state handlers
5. **Preprocessor**: Converts QP syntax → Mermaid for visualization

### QP-Specific Syntax Examples
```mermaid
state On {
    entry: BSP_ledOn()
    exit: BSP_ledOff()
    
    [*] --> Active
    
    state Active {
        Active --> Active : BUTTON [count < 5] / count++
    }
}
```

### Development Commands
```bash
cd QP-QK-SDK/tools/vscode-extensions/qp-mermaid
npm install          # Install dependencies
npm run compile      # Compile TypeScript
npm run compile-web  # Compile for web (GitHub.dev)
# Press F5 in VS Code to test
```

## Current Focus
The extension now supports hierarchical state machines and complex real-world examples. The architecture is:
- **Preprocessor**: Handles QP→Mermaid conversion
- **Parser**: Enhanced with hierarchical state support
- **Generator**: Produces production-ready QP/C code
- **Validator**: Comprehensive semantic checking

## Next Steps
1. Test Phase 2 features with complex examples
2. Add remaining features (IntelliSense, tests)
3. Consider Phase 3: Custom visual editor
4. Prepare for VS Code Marketplace publication

## Important Context
- Working for Murray Kopit (murr2k@gmail.com)
- Goal: Modern web-based alternative to desktop QM tool
- Target: Embedded systems developers using QP framework
- Must maintain compatibility with existing QP/C code patterns

## Git Status
- Repository: https://github.com/murr2k/qp
- Latest tag: phase1-complete
- Branch: main
- Uncommitted changes: Phase 2 implementation

Use this prompt to quickly get back up to speed with the project state and continue development.