import { StateMachine, State } from '../types/stateMachine';

interface PreprocessResult {
    mermaidDiagram: string;
    metadata: string;
}

/**
 * Preprocesses QP-enhanced Mermaid syntax into standard Mermaid for visualization
 * while preserving QP-specific information in metadata comments
 */
export class MermaidPreprocessor {
    private metadata: Map<string, any> = new Map();
    private stateCounter = 0;

    preprocess(content: string): PreprocessResult {
        this.metadata.clear();
        this.stateCounter = 0;

        const lines = content.split('\n');
        const outputLines: string[] = [];
        let inStateBlock = false;
        let currentState: string | null = null;
        let stateIndent = '';
        let skipNextLine = false;

        for (let i = 0; i < lines.length; i++) {
            if (skipNextLine) {
                skipNextLine = false;
                continue;
            }

            const line = lines[i];
            const trimmedLine = line.trim();

            // Handle state blocks with entry/exit actions
            if (trimmedLine.match(/^state\s+(\w+)\s*{$/)) {
                const match = trimmedLine.match(/^state\s+(\w+)\s*{$/);
                currentState = match![1];
                inStateBlock = true;
                stateIndent = line.match(/^\s*/)?.[0] || '';
                this.metadata.set(currentState, { entry: [], exit: [], internal: [] });
                outputLines.push(line);
                continue;
            }

            // Handle closing brace
            if (inStateBlock && trimmedLine === '}') {
                inStateBlock = false;
                currentState = null;
                outputLines.push(line);
                continue;
            }

            // Handle entry/exit actions inside state blocks
            if (inStateBlock && currentState) {
                if (trimmedLine.startsWith('entry:')) {
                    const action = trimmedLine.substring(6).trim();
                    this.metadata.get(currentState).entry.push(action);
                    // Add a comment in the output
                    outputLines.push(`${stateIndent}    %% entry: ${action}`);
                    continue;
                } else if (trimmedLine.startsWith('exit:')) {
                    const action = trimmedLine.substring(5).trim();
                    this.metadata.get(currentState).exit.push(action);
                    outputLines.push(`${stateIndent}    %% exit: ${action}`);
                    continue;
                } else if (trimmedLine.includes(':') && !trimmedLine.includes('-->')) {
                    // Internal transition
                    const match = trimmedLine.match(/:\s*(\w+)\s*(?:\/\s*(.+))?/);
                    if (match) {
                        this.metadata.get(currentState).internal.push({
                            event: match[1],
                            action: match[2]
                        });
                        outputLines.push(`${stateIndent}    %% internal: ${trimmedLine}`);
                        continue;
                    }
                }
            }

            // Handle transitions with guards and actions
            if (trimmedLine.includes('-->')) {
                const processedLine = this.processTransition(line);
                outputLines.push(processedLine);
                continue;
            }

            // Handle history states
            if (trimmedLine.includes('[H]') || trimmedLine.includes('[H*]')) {
                const historyType = trimmedLine.includes('[H*]') ? 'deep' : 'shallow';
                const processedLine = line.replace(/\[H\*?\]/, `History_${++this.stateCounter}`);
                outputLines.push(processedLine);
                
                // Store history metadata
                const stateName = `History_${this.stateCounter}`;
                this.metadata.set(stateName, { type: 'history', historyType });
                continue;
            }

            // Pass through other lines
            outputLines.push(line);
        }

        // Add metadata as comments at the end
        const metadataJson = JSON.stringify(Object.fromEntries(this.metadata), null, 2);
        const metadataComment = `\n%%{qp-metadata:\n${metadataJson}\n}%%`;

        return {
            mermaidDiagram: outputLines.join('\n'),
            metadata: metadataComment
        };
    }

    private processTransition(line: string): string {
        // Extract and simplify complex transitions
        // Pattern: Source --> Target : Event [guard] / action
        const match = line.match(/^(\s*)(\w+)\s*-->\s*(\w+)\s*:\s*([^[/]+)(?:\s*\[([^\]]+)\])?(?:\s*\/\s*(.+))?$/);
        
        if (!match) {
            return line; // Return as-is if pattern doesn't match
        }

        const [_, indent, source, target, event, guard, action] = match;
        const trimmedEvent = event.trim();

        // Store guard and action in metadata
        const transitionKey = `${source}_to_${target}_${trimmedEvent}`;
        const transitionMeta: any = { source, target, event: trimmedEvent };
        
        if (guard) {
            transitionMeta.guard = guard.trim();
        }
        if (action) {
            transitionMeta.action = action.trim();
        }

        this.metadata.set(transitionKey, transitionMeta);

        // Create simplified transition for Mermaid
        let simplifiedLabel = trimmedEvent;
        if (guard || action) {
            // Add indicators that there's more info
            simplifiedLabel += ' ✓';
        }

        return `${indent}${source} --> ${target} : ${simplifiedLabel}`;
    }

    /**
     * Extracts metadata from preprocessed content
     */
    static extractMetadata(content: string): any {
        const metadataMatch = content.match(/%%{qp-metadata:\n(.+)\n}%%/s);
        if (metadataMatch) {
            try {
                return JSON.parse(metadataMatch[1]);
            } catch (e) {
                console.error('Failed to parse QP metadata:', e);
                return {};
            }
        }
        return {};
    }

    /**
     * Combines Mermaid diagram with QP metadata for full reconstruction
     */
    static reconstruct(mermaidDiagram: string, metadata: any): string {
        // This would reconstruct the original QP syntax from Mermaid + metadata
        // Used when loading files or preparing for code generation
        let content = mermaidDiagram;

        // Remove metadata comment
        content = content.replace(/\n%%{qp-metadata:[\s\S]+}%%/, '');

        // TODO: Implement full reconstruction logic
        // This would reverse the preprocessing, adding back entry/exit actions, etc.

        return content;
    }
}