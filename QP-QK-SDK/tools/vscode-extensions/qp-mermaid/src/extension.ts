import * as vscode from 'vscode';
import { PreviewPanel } from './webview/previewPanel';
import { QpMermaidParser } from './parser/qpMermaidParser';
import { QpCodeGenerator } from './generator/qpCodeGenerator';

export function activate(context: vscode.ExtensionContext) {
    console.log('QP Mermaid extension is now active!');

    // Register preview command
    const previewCommand = vscode.commands.registerCommand('qp-mermaid.preview', () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            vscode.window.showErrorMessage('No active editor found');
            return;
        }

        if (!editor.document.fileName.endsWith('.qp.mmd')) {
            vscode.window.showErrorMessage('Please open a .qp.mmd file');
            return;
        }

        PreviewPanel.createOrShow(context.extensionUri, editor.document);
    });

    // Register code generation command
    const generateCodeCommand = vscode.commands.registerCommand('qp-mermaid.generateCode', async () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            vscode.window.showErrorMessage('No active editor found');
            return;
        }

        if (!editor.document.fileName.endsWith('.qp.mmd')) {
            vscode.window.showErrorMessage('Please open a .qp.mmd file');
            return;
        }

        try {
            // Parse the Mermaid content
            const parser = new QpMermaidParser();
            const stateMachine = parser.parse(editor.document.getText());

            // Generate code
            const generator = new QpCodeGenerator();
            const config = vscode.workspace.getConfiguration('qp-mermaid');
            const targetLanguage = config.get<string>('codeGeneration.targetLanguage', 'C');
            const includeComments = config.get<boolean>('codeGeneration.includeComments', true);

            const generatedCode = generator.generate(stateMachine, {
                language: targetLanguage as 'C' | 'C++',
                includeComments
            });

            // Create output files
            const workspaceFolder = vscode.workspace.getWorkspaceFolder(editor.document.uri);
            if (!workspaceFolder) {
                vscode.window.showErrorMessage('No workspace folder found');
                return;
            }

            // Save generated files
            const baseName = editor.document.fileName.replace(/\.qp\.mmd$/, '');
            const headerPath = vscode.Uri.file(`${baseName}.h`);
            const sourcePath = vscode.Uri.file(`${baseName}.c`);

            await vscode.workspace.fs.writeFile(headerPath, Buffer.from(generatedCode.header));
            await vscode.workspace.fs.writeFile(sourcePath, Buffer.from(generatedCode.source));

            vscode.window.showInformationMessage(`Generated ${stateMachine.name}.h and ${stateMachine.name}.c`);

            // Open the generated source file
            const doc = await vscode.workspace.openTextDocument(sourcePath);
            await vscode.window.showTextDocument(doc);

        } catch (error) {
            vscode.window.showErrorMessage(`Code generation failed: ${error}`);
        }
    });

    // Register export to QM command
    const exportToQmCommand = vscode.commands.registerCommand('qp-mermaid.exportToQM', async () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            vscode.window.showErrorMessage('No active editor found');
            return;
        }

        // TODO: Implement QM export functionality
        vscode.window.showInformationMessage('Export to QM format - Coming soon!');
    });

    // Register document change listener for live preview updates
    const documentChangeListener = vscode.workspace.onDidChangeTextDocument((e) => {
        if (e.document.fileName.endsWith('.qp.mmd')) {
            PreviewPanel.update(e.document);
        }
    });

    // Register custom editor provider for .qp.mmd files
    const customEditorProvider = vscode.window.registerCustomEditorProvider(
        'qp-mermaid.editor',
        {
            async openCustomDocument(uri: vscode.Uri): Promise<vscode.CustomDocument> {
                const document = await vscode.workspace.openTextDocument(uri);
                return {
                    uri,
                    dispose: () => {}
                };
            },
            async resolveCustomEditor(
                document: vscode.CustomDocument,
                webviewPanel: vscode.WebviewPanel,
                _token: vscode.CancellationToken
            ): Promise<void> {
                // Set up the webview
                webviewPanel.webview.options = {
                    enableScripts: true
                };

                // Load content
                const textDocument = await vscode.workspace.openTextDocument(document.uri);
                PreviewPanel.setupWebview(webviewPanel, context.extensionUri, textDocument);
            }
        },
        {
            webviewOptions: {
                retainContextWhenHidden: true
            }
        }
    );

    context.subscriptions.push(
        previewCommand,
        generateCodeCommand,
        exportToQmCommand,
        documentChangeListener,
        customEditorProvider
    );
}

export function deactivate() {
    console.log('QP Mermaid extension deactivated');
}