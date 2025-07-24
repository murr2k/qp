import * as vscode from 'vscode';
import { MermaidPreprocessor } from '../preprocessor/mermaidPreprocessor';

export class PreviewPanel {
    public static currentPanel: PreviewPanel | undefined;
    private static readonly viewType = 'qpMermaidPreview';

    private readonly _panel: vscode.WebviewPanel;
    private readonly _extensionUri: vscode.Uri;
    private _disposables: vscode.Disposable[] = [];
    private _document: vscode.TextDocument;

    public static createOrShow(extensionUri: vscode.Uri, document: vscode.TextDocument) {
        const column = vscode.window.activeTextEditor
            ? vscode.window.activeTextEditor.viewColumn
            : undefined;

        // If we already have a panel, show it
        if (PreviewPanel.currentPanel) {
            PreviewPanel.currentPanel._panel.reveal(column);
            PreviewPanel.currentPanel._document = document;
            PreviewPanel.currentPanel._update();
            return;
        }

        // Otherwise, create a new panel
        const panel = vscode.window.createWebviewPanel(
            PreviewPanel.viewType,
            'QP State Machine Preview',
            column || vscode.ViewColumn.One,
            {
                enableScripts: true,
                localResourceRoots: [vscode.Uri.joinPath(extensionUri, 'media')]
            }
        );

        PreviewPanel.currentPanel = new PreviewPanel(panel, extensionUri, document);
    }

    public static update(document: vscode.TextDocument) {
        if (PreviewPanel.currentPanel) {
            PreviewPanel.currentPanel._document = document;
            PreviewPanel.currentPanel._update();
        }
    }

    public static setupWebview(panel: vscode.WebviewPanel, extensionUri: vscode.Uri, document: vscode.TextDocument) {
        const previewPanel = new PreviewPanel(panel, extensionUri, document);
        PreviewPanel.currentPanel = previewPanel;
    }

    private constructor(panel: vscode.WebviewPanel, extensionUri: vscode.Uri, document: vscode.TextDocument) {
        this._panel = panel;
        this._extensionUri = extensionUri;
        this._document = document;

        // Set the webview's initial html content
        this._update();

        // Listen for when the panel is disposed
        this._panel.onDidDispose(() => this.dispose(), null, this._disposables);

        // Update the content based on view changes
        this._panel.onDidChangeViewState(
            e => {
                if (this._panel.visible) {
                    this._update();
                }
            },
            null,
            this._disposables
        );

        // Handle messages from the webview
        this._panel.webview.onDidReceiveMessage(
            message => {
                switch (message.command) {
                    case 'alert':
                        vscode.window.showErrorMessage(message.text);
                        return;
                }
            },
            null,
            this._disposables
        );
    }

    public dispose() {
        PreviewPanel.currentPanel = undefined;

        // Clean up our resources
        this._panel.dispose();

        while (this._disposables.length) {
            const x = this._disposables.pop();
            if (x) {
                x.dispose();
            }
        }
    }

    private _update() {
        const webview = this._panel.webview;
        const fileName = this._document.fileName.split('/').pop() || 'untitled';
        this._panel.title = 'QP Preview: ' + fileName;
        this._panel.webview.html = this._getHtmlForWebview(webview);
    }

    private _getHtmlForWebview(webview: vscode.Webview) {
        // Get the theme from configuration
        const config = vscode.workspace.getConfiguration('qp-mermaid');
        const theme = config.get<string>('preview.theme', 'default');

        let mermaidContent = this._document.getText();
        
        // Remove markdown code fences if present
        mermaidContent = mermaidContent.replace(/^```mermaid\n/, '');
        mermaidContent = mermaidContent.replace(/\n```$/, '');
        mermaidContent = mermaidContent.trim();

        // Preprocess QP-specific syntax
        const preprocessor = new MermaidPreprocessor();
        const preprocessed = preprocessor.preprocess(mermaidContent);
        mermaidContent = preprocessed.mermaidDiagram;

        return `<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>QP State Machine Preview</title>
    <script src="https://cdn.jsdelivr.net/npm/mermaid@10/dist/mermaid.min.js"></script>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: var(--vscode-editor-background);
            color: var(--vscode-editor-foreground);
        }
        #diagram {
            text-align: center;
            background: white;
            border-radius: 8px;
            padding: 20px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
        }
        .error {
            color: var(--vscode-errorForeground);
            background-color: var(--vscode-inputValidation-errorBackground);
            padding: 10px;
            border-radius: 4px;
            margin: 10px 0;
        }
        .controls {
            margin-bottom: 20px;
            text-align: center;
        }
        button {
            background-color: var(--vscode-button-background);
            color: var(--vscode-button-foreground);
            border: none;
            padding: 6px 14px;
            margin: 0 5px;
            border-radius: 4px;
            cursor: pointer;
        }
        button:hover {
            background-color: var(--vscode-button-hoverBackground);
        }
    </style>
</head>
<body>
    <div class="controls">
        <button onclick="zoomIn()">Zoom In</button>
        <button onclick="zoomOut()">Zoom Out</button>
        <button onclick="resetZoom()">Reset Zoom</button>
        <button onclick="exportSvg()">Export SVG</button>
    </div>
    <div id="diagram">
        <pre class="mermaid">
${mermaidContent}
        </pre>
    </div>
    
    <script>
        let currentZoom = 1;
        const vscode = acquireVsCodeApi();
        
        // Initialize Mermaid with the selected theme
        mermaid.initialize({ 
            startOnLoad: true,
            theme: '${theme}',
            securityLevel: 'loose',
            flowchart: {
                useMaxWidth: true,
                htmlLabels: true
            }
        });

        // Zoom functions
        function zoomIn() {
            currentZoom *= 1.2;
            applyZoom();
        }

        function zoomOut() {
            currentZoom *= 0.8;
            applyZoom();
        }

        function resetZoom() {
            currentZoom = 1;
            applyZoom();
        }

        function applyZoom() {
            const svg = document.querySelector('#diagram svg');
            if (svg) {
                svg.style.transform = \`scale(\${currentZoom})\`;
                svg.style.transformOrigin = 'center';
            }
        }

        function exportSvg() {
            const svg = document.querySelector('#diagram svg');
            if (svg) {
                const svgData = new XMLSerializer().serializeToString(svg);
                const blob = new Blob([svgData], { type: 'image/svg+xml' });
                const url = URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.href = url;
                a.download = 'state-machine.svg';
                a.click();
                URL.revokeObjectURL(url);
            }
        }

        // Handle Mermaid rendering errors
        window.addEventListener('load', () => {
            const mermaidError = document.querySelector('.mermaid-error');
            if (mermaidError) {
                vscode.postMessage({
                    command: 'alert',
                    text: 'Error rendering Mermaid diagram: ' + mermaidError.textContent
                });
            }
        });
    </script>
</body>
</html>`;
    }
}