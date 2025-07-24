# GitHub.dev Setup Guide for QP Mermaid Extension

This guide explains how to use the QP Mermaid extension in GitHub.dev for browser-based state machine development.

## What is GitHub.dev?

GitHub.dev is a web-based version of VS Code that runs entirely in your browser. You can access it by:
- Pressing `.` (period key) in any GitHub repository
- Changing `github.com` to `github.dev` in any repo URL
- Going to https://github.dev

## Installation in GitHub.dev

### Option 1: From VS Code Marketplace (Once Published)
1. Open github.dev in your repository
2. Click the Extensions icon in the sidebar (Ctrl+Shift+X)
3. Search for "QP Mermaid"
4. Click Install

### Option 2: Install from VSIX (Development)
1. Build the extension locally:
   ```bash
   cd QP-QK-SDK/tools/vscode-extensions/qp-mermaid
   npm install
   npm run package-web
   ```
2. Upload the generated `.vsix` file to your repository
3. In github.dev, open Command Palette (Ctrl+Shift+P)
4. Run "Extensions: Install from VSIX..."
5. Select the uploaded `.vsix` file

## Features Available in GitHub.dev

✅ **Full Support:**
- Syntax highlighting for `.qp.mmd` files
- Live preview of state diagrams
- Code generation to QP/C
- All editor commands and keybindings
- Configuration settings

❌ **Not Available:**
- Direct file system compilation
- Terminal commands
- Debugging

## Usage Example

1. Create a new file: `my_state_machine.qp.mmd`
2. Add your state machine:
   ```mermaid
   stateDiagram-v2
       [*] --> Idle
       Idle --> Active : START
       Active --> Idle : STOP
   ```
3. Press `Ctrl+Shift+V` to preview
4. Use Command Palette to generate code

## Tips for GitHub.dev

1. **Auto-save**: Enable auto-save for seamless experience
2. **Split view**: Use split editor to see code and preview side-by-side
3. **Commit changes**: Use Source Control panel to commit directly from browser
4. **Keyboard shortcuts**: All standard VS Code shortcuts work

## Limitations and Workarounds

| Limitation | Workaround |
|------------|------------|
| No terminal access | Use GitHub Actions for building |
| Can't run compilers | Generate code and download for local compilation |
| Limited to web APIs | Extension designed to work within browser constraints |

## Integration with GitHub Actions

Create `.github/workflows/build-qp.yml`:

```yaml
name: Build QP State Machines
on:
  push:
    paths:
      - '**.qp.mmd'
jobs:
  generate:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Generate QP Code
        run: |
          # Add your build commands here
          echo "Building QP code from Mermaid diagrams"
```

## Best Practices

1. **Version Control**: Commit `.qp.mmd` files alongside generated code
2. **Documentation**: Use Mermaid diagrams in README files
3. **Collaboration**: Share state machine designs via GitHub URLs
4. **Code Review**: Review state machines visually in PRs

## Troubleshooting

### Extension not loading
- Ensure you're using a modern browser (Chrome, Edge, Firefox)
- Clear browser cache and reload
- Check browser console for errors

### Preview not working
- Verify file has `.qp.mmd` extension
- Check for syntax errors in Mermaid diagram
- Try closing and reopening preview

### Code generation issues
- Ensure valid QP syntax in diagram
- Check browser console for detailed errors
- Verify all required annotations are present

## Future Enhancements

- Direct GitHub Gist integration
- Collaborative editing support
- Cloud-based compilation
- AI-powered state machine suggestions

The QP Mermaid extension brings modern, web-based state machine development to embedded systems programming!