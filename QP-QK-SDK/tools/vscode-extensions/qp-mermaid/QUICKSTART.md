# QP Mermaid Extension Quick Start

## Installation

1. Install the extension from VS Code Marketplace (once published)
2. Or build from source:
   ```bash
   cd QP-QK-SDK/tools/vscode-extensions/qp-mermaid
   npm install
   npm run compile
   code --install-extension qp-mermaid-0.1.0.vsix
   ```

## Your First State Machine

1. Create a new file called `blinky.qp.mmd`

2. Copy this starter template:

```mermaid
stateDiagram-v2
    %% QP Active Object: Blinky
    %% Priority: 1
    %% Stack Size: 512
    
    [*] --> Off : @Q_INIT_SIG
    
    state Off {
        entry: BSP_ledOff()
    }
    
    state On {
        entry: BSP_ledOn()
    }
    
    Off --> On : BUTTON_PRESSED
    On --> Off : BUTTON_RELEASED
```

3. Press `Ctrl+Shift+V` (or `Cmd+Shift+V` on Mac) to preview

4. Click "Generate C Code" in the editor toolbar

## Tips

- Use `%%` for comments
- Add multiple entry/exit actions on separate lines
- Use `[guard]` syntax for conditional transitions
- Add `/action()` after events for transition actions
- Nest states for hierarchical state machines

## Next Steps

- Read the full README for advanced features
- Check examples in the `examples/` directory
- Join the QP community for support