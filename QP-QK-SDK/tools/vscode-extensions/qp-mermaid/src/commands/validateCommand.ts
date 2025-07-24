import * as vscode from 'vscode';
import { QpMermaidParser } from '../parser/qpMermaidParser';
import { StateMachineValidator } from '../validator/stateMachineValidator';

export function registerValidateCommand(context: vscode.ExtensionContext) {
    const validateCommand = vscode.commands.registerCommand('qp-mermaid.validate', async () => {
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
            // Parse the state machine
            const parser = new QpMermaidParser();
            const stateMachine = parser.parse(editor.document.getText());

            // Validate it
            const validator = new StateMachineValidator();
            const errors = validator.validate(stateMachine);

            // Create diagnostics
            const diagnostics: vscode.Diagnostic[] = [];
            const diagnosticCollection = vscode.languages.createDiagnosticCollection('qp-mermaid');

            for (const error of errors) {
                const severity = error.severity === 'error' 
                    ? vscode.DiagnosticSeverity.Error
                    : error.severity === 'warning'
                    ? vscode.DiagnosticSeverity.Warning
                    : vscode.DiagnosticSeverity.Information;

                // For now, put all diagnostics at line 0
                // In future, we could enhance parser to track line numbers
                const range = new vscode.Range(0, 0, 0, 0);
                
                const diagnostic = new vscode.Diagnostic(
                    range,
                    error.message,
                    severity
                );

                if (error.location) {
                    diagnostic.source = error.location.state || error.location.transition;
                }

                diagnostics.push(diagnostic);
            }

            // Set diagnostics
            diagnosticCollection.set(editor.document.uri, diagnostics);

            // Show summary
            const errorCount = errors.filter(e => e.severity === 'error').length;
            const warningCount = errors.filter(e => e.severity === 'warning').length;
            
            if (errorCount === 0 && warningCount === 0) {
                vscode.window.showInformationMessage('✓ State machine validation passed!');
            } else {
                vscode.window.showWarningMessage(
                    `Validation found ${errorCount} errors and ${warningCount} warnings`
                );
            }

            // Also output to channel
            const outputChannel = vscode.window.createOutputChannel('QP Mermaid Validation');
            outputChannel.clear();
            outputChannel.appendLine('=== QP State Machine Validation Results ===');
            outputChannel.appendLine(`File: ${editor.document.fileName}`);
            outputChannel.appendLine(`State Machine: ${stateMachine.name}`);
            outputChannel.appendLine('');

            if (errors.length === 0) {
                outputChannel.appendLine('✓ No issues found!');
            } else {
                for (const error of errors) {
                    const icon = error.severity === 'error' ? '❌' : error.severity === 'warning' ? '⚠️' : 'ℹ️';
                    const location = error.location 
                        ? ` [${error.location.state || error.location.transition || ''}]`
                        : '';
                    outputChannel.appendLine(`${icon} ${error.message}${location}`);
                }
            }

            outputChannel.show();

        } catch (error) {
            vscode.window.showErrorMessage(`Validation failed: ${error}`);
        }
    });

    context.subscriptions.push(validateCommand);

    // Also run validation on save
    const onSaveValidation = vscode.workspace.onDidSaveTextDocument((document) => {
        if (document.fileName.endsWith('.qp.mmd')) {
            vscode.commands.executeCommand('qp-mermaid.validate');
        }
    });

    context.subscriptions.push(onSaveValidation);
}