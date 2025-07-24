# QP Mermaid State Machine Extension

Visual state machine editor for the QP framework using Mermaid diagrams. This VS Code extension enables you to design state machines visually and generate QP/C code automatically.

## Features

- **Visual State Machine Design**: Create state machines using extended Mermaid syntax
- **QP-Specific Extensions**: Support for QP events, guards, actions, and hierarchical states
- **Code Generation**: Automatically generate QP/C code from your diagrams
- **Live Preview**: See your state machine rendered in real-time
- **GitHub.dev Compatible**: Works in browser-based VS Code
- **Web Extension**: Full support for github.dev and vscode.dev

## Getting Started

1. Create a new file with `.qp.mmd` extension
2. Start designing your state machine using Mermaid syntax
3. Use the preview command to see your diagram
4. Generate QP/C code when ready

## Example

```mermaid
stateDiagram-v2
    %% QP Active Object: Blinky
    %% Priority: 1
    %% Stack Size: 512
    
    [*] --> Off : @Q_INIT_SIG
    
    state Off {
        entry: BSP_ledOff(0)
        entry: QTimeEvt_armX(&me->timeEvt, BSP_TICKS_PER_SEC/2, 0U)
    }
    
    state On {
        entry: BSP_ledOn(0)
        entry: QTimeEvt_armX(&me->timeEvt, BSP_TICKS_PER_SEC/2, 0U)
    }
    
    Off --> On : TIMEOUT
    On --> Off : TIMEOUT
```

## Commands

- **QP: Preview State Machine** (`Ctrl+Shift+V`): Open live preview
- **QP: Generate C Code**: Generate QP/C code from diagram
- **QP: Export to QM Format**: Export to QM tool format (coming soon)

## QP Extensions to Mermaid

### State Actions
```
state StateName {
    entry: action1()
    entry: action2()
    exit: cleanup()
}
```

### Transitions with Guards and Actions
```
State1 --> State2 : EVENT / action() [guard]
```

### Internal Transitions
```
state Active {
    : TICK / updateCounter()
}
```

### Hierarchical States
```
state Operational {
    [*] --> SubState1
    
    state SubState1 {
        entry: doSomething()
    }
    
    state SubState2 {
        entry: doSomethingElse()
    }
}
```

## Configuration

Configure the extension in VS Code settings:

- `qp-mermaid.codeGeneration.targetLanguage`: Choose between "C" and "C++"
- `qp-mermaid.codeGeneration.includeComments`: Include comments in generated code
- `qp-mermaid.preview.theme`: Mermaid diagram theme (default, dark, forest, neutral)

## Using in GitHub.dev

1. Open any GitHub repository in github.dev (press `.` in any repo)
2. Install the QP Mermaid extension from the Extensions view
3. Create `.qp.mmd` files and start designing!

The extension works fully in the browser without any server-side components.

## Development

To build and test the extension locally:

```bash
npm install
npm run compile-web  # For web extension
npm run compile      # For desktop VS Code
```

For web extension development:
```bash
npm run watch-web
```

Then press F5 in VS Code to launch a new Extension Development Host window.

## License

This extension is part of the QP-QK SDK project.

## Contributing

Contributions are welcome! Please submit issues and pull requests on GitHub.