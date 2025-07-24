# Phase 1 Implementation Restart Prompt

## Command
```bash
npx claude-flow@alpha
```

## Prompt to Continue Phase 1

```
I'm continuing work on the QP-QK SDK project in /mnt/c/Users/murr2/projects/qp. 

We've completed extensive analysis and planning for a web-based visual state machine editor that works in GitHub.dev. The key decision was to use Mermaid as the foundation for a VS Code web extension.

Please implement Phase 1: Mermaid QP Extension as documented in:
- QP-QK-SDK/docs/VSCODE_EXTENSION_EVALUATION.md (see Phase 1: Mermaid QP Extension section)
- QP-QK-SDK/docs/GITHUB_DEV_VISUAL_EDITOR_PROPOSAL.md (see Implementation Plan)
- QP-QK-SDK/examples/mermaid/blinky_state_machine.qp.mmd (example syntax)

Phase 1 Goals (4-6 weeks):
1. Fork/create a VS Code extension based on Mermaid Chart
2. Add QP-specific syntax extensions to Mermaid state diagrams
3. Implement basic QP/C code generation from Mermaid diagrams
4. Ensure full compatibility with GitHub.dev (web extension)
5. Create initial documentation and examples

Start by:
1. Creating the extension project structure at QP-QK-SDK/tools/vscode-extensions/qp-mermaid/
2. Setting up the basic VS Code extension with Mermaid support
3. Adding QP-specific syntax parsing for state machines
4. Implementing a simple code generator that converts Mermaid diagrams to QP/C code

Focus on getting a working proof-of-concept that can visualize and generate code for the blinky example.
```

## Context Files to Review

When restarting, these files contain the key information:

1. **Planning Documents**:
   - `QP-QK-SDK/docs/QM_ANALYSIS.md` - Analysis of existing QM tool
   - `QP-QK-SDK/docs/GITHUB_DEV_VISUAL_EDITOR_PROPOSAL.md` - Implementation strategy
   - `QP-QK-SDK/docs/VSCODE_EXTENSION_EVALUATION.md` - Extension evaluation and roadmap

2. **Example Syntax**:
   - `QP-QK-SDK/examples/mermaid/blinky_state_machine.qp.mmd` - Proposed Mermaid syntax

3. **Integration Points**:
   - `QP-QK-SDK/docs/QTOOLS_INTEGRATION_ANALYSIS.md` - How this fits with other tools
   - `QP-QK-SDK/FUTURE_FEATURES.md` - Overall feature roadmap

## Quick Architecture Reference

```
qp-mermaid-extension/
├── package.json          # Extension manifest
├── src/
│   ├── extension.ts      # Main entry point
│   ├── parser/           # Mermaid to QP parsing
│   ├── generator/        # Code generation
│   └── webview/          # Visual editor (future)
└── examples/             # Test cases
```

## Key Technical Decisions Made

1. **Platform**: GitHub.dev (web-based VS Code)
2. **Foundation**: Mermaid state diagrams
3. **Language**: TypeScript/JavaScript (web compatible)
4. **Code Generation**: Template-based QP/C generation
5. **Distribution**: VS Code Marketplace

## Success Criteria for Phase 1

- [ ] Extension loads in GitHub.dev
- [ ] Can parse QP-enhanced Mermaid syntax
- [ ] Generates valid QP/C code for simple state machines
- [ ] Includes example for Blinky
- [ ] Basic documentation complete

This prompt will help you quickly resume work on implementing the visual state machine editor for the QP-QK SDK.