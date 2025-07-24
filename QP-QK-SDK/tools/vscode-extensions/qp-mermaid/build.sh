#!/bin/bash

echo "Building QP Mermaid Extension..."

# Clean previous build
rm -rf out/
rm -f *.vsix

# Install dependencies if needed
if [ ! -d "node_modules" ]; then
    echo "Installing dependencies..."
    npm install
fi

# Compile TypeScript
echo "Compiling TypeScript..."
npm run compile

# Package extension
echo "Packaging extension..."
npx vsce package

echo "Build complete! Extension packaged as qp-mermaid-*.vsix"
echo "To install: code --install-extension qp-mermaid-*.vsix"