import { StateMachine, State, Transition, Event, DataMember } from '../types/stateMachine';

export class QpMermaidParser {
    private stateMachine: StateMachine;
    private currentIndentLevel: number = 0;
    private stateStack: State[] = [];

    constructor() {
        this.stateMachine = {
            name: 'StateMachine',
            type: 'QActive',
            events: [],
            states: [],
            initialState: '',
            dataMembers: []
        };
    }

    parse(content: string): StateMachine {
        const lines = content.split('\n');
        let inDiagram = false;
        let inMetadata = false;
        let metadataContent = '';

        for (let i = 0; i < lines.length; i++) {
            const line = lines[i].trim();

            // Skip empty lines and comments
            if (!line || line.startsWith('%%') && !line.startsWith('%%{')) {
                continue;
            }

            // Handle metadata blocks
            if (line.startsWith('%%{')) {
                inMetadata = true;
                metadataContent = '';
                continue;
            }
            if (line === '}%%' && inMetadata) {
                inMetadata = false;
                this.parseMetadata(metadataContent);
                continue;
            }
            if (inMetadata) {
                metadataContent += line + '\n';
                continue;
            }

            // Handle diagram start
            if (line.startsWith('stateDiagram-v2') || line.startsWith('stateDiagram')) {
                inDiagram = true;
                continue;
            }

            if (!inDiagram) {
                continue;
            }

            // Parse diagram content
            this.parseLine(line);
        }

        // Post-process to set up parent-child relationships
        this.setupHierarchy();

        return this.stateMachine;
    }

    private parseLine(line: string) {
        // Initial state transition
        if (line.includes('[*]') && line.includes('-->')) {
            const match = line.match(/\[\*\]\s*-->\s*(\w+)(?:\s*:\s*(.+))?/);
            if (match) {
                this.stateMachine.initialState = match[1];
                if (match[2] && match[2].includes('@Q_INIT_SIG')) {
                    // Mark as QP initial transition
                }
            }
            return;
        }

        // State with actions
        if (line.startsWith('state ') && line.includes('{')) {
            const match = line.match(/state\s+(\w+)\s*{/);
            if (match) {
                const state = this.findOrCreateState(match[1]);
                this.stateStack.push(state);
            }
            return;
        }

        // Closing brace
        if (line === '}') {
            this.stateStack.pop();
            return;
        }

        // Entry/exit actions
        if (line.startsWith('entry:')) {
            const action = line.substring(6).trim();
            if (this.stateStack.length > 0) {
                this.stateStack[this.stateStack.length - 1].entryActions.push(action);
            }
            return;
        }
        if (line.startsWith('exit:')) {
            const action = line.substring(5).trim();
            if (this.stateStack.length > 0) {
                this.stateStack[this.stateStack.length - 1].exitActions.push(action);
            }
            return;
        }

        // Transitions
        if (line.includes('-->')) {
            this.parseTransition(line);
            return;
        }

        // Internal transitions
        if (line.includes(':') && !line.includes('-->') && this.stateStack.length > 0) {
            const match = line.match(/:\s*(\w+)\s*\/\s*(.+)/);
            if (match) {
                const currentState = this.stateStack[this.stateStack.length - 1];
                currentState.transitions.push({
                    event: match[1],
                    target: currentState.name,
                    action: match[2],
                    isInternal: true
                });
            }
        }
    }

    private parseTransition(line: string) {
        // Parse transitions like: State1 --> State2 : EVENT / action() [guard]
        const match = line.match(/(\w+)\s*-->\s*(\w+)(?:\s*:\s*(\w+)(?:\s*\/\s*([^[]+))?(?:\s*\[([^\]]+)\])?)?/);
        if (match) {
            const [_, source, target, event, action, guard] = match;
            const sourceState = this.findOrCreateState(source);
            
            // Ensure target state exists
            this.findOrCreateState(target);

            const transition: Transition = {
                event: event || '',
                target: target,
                action: action?.trim(),
                guard: guard?.trim()
            };

            sourceState.transitions.push(transition);
        }
    }

    private parseMetadata(content: string) {
        try {
            // Try to parse as JSON first
            const metadata = JSON.parse(content);
            
            if (metadata.init?.qp) {
                const qp = metadata.init.qp;
                this.stateMachine.name = qp.activeObject || this.stateMachine.name;
                this.stateMachine.type = qp.extends || this.stateMachine.type;
                this.stateMachine.priority = qp.priority;
                this.stateMachine.stackSize = qp.stackSize;
            }

            if (metadata.events) {
                this.stateMachine.events = metadata.events;
            }

            if (metadata.data) {
                this.stateMachine.dataMembers = metadata.data;
            }
        } catch (e) {
            // If not JSON, try to parse key-value pairs
            const lines = content.split('\n');
            for (const line of lines) {
                if (line.includes('Active Object:')) {
                    this.stateMachine.name = line.split(':')[1].trim();
                } else if (line.includes('Priority:')) {
                    this.stateMachine.priority = parseInt(line.split(':')[1].trim());
                } else if (line.includes('Stack Size:')) {
                    this.stateMachine.stackSize = parseInt(line.split(':')[1].trim());
                }
            }
        }
    }

    private findOrCreateState(name: string): State {
        let state = this.findState(name);
        if (!state) {
            state = {
                name: name,
                entryActions: [],
                exitActions: [],
                transitions: [],
                children: []
            };
            
            // Add to appropriate parent or root
            if (this.stateStack.length > 0) {
                const parent = this.stateStack[this.stateStack.length - 1];
                state.parent = parent.name;
                parent.children.push(state);
            } else {
                this.stateMachine.states.push(state);
            }
        }
        return state;
    }

    private findState(name: string): State | undefined {
        // Search in all states recursively
        const searchInStates = (states: State[]): State | undefined => {
            for (const state of states) {
                if (state.name === name) {
                    return state;
                }
                const found = searchInStates(state.children);
                if (found) {
                    return found;
                }
            }
            return undefined;
        };

        return searchInStates(this.stateMachine.states);
    }

    private setupHierarchy() {
        // Flatten all states for easier processing
        const allStates: State[] = [];
        const collectStates = (states: State[]) => {
            for (const state of states) {
                allStates.push(state);
                collectStates(state.children);
            }
        };
        collectStates(this.stateMachine.states);

        // Set up initial transitions for composite states
        for (const state of allStates) {
            if (state.children.length > 0) {
                // Find initial child state
                const initialChild = state.children.find(child => {
                    return this.stateMachine.states.some(s => 
                        s.transitions.some(t => 
                            t.target === child.name && t.event === '@Q_INIT_SIG'
                        )
                    );
                });
                
                if (initialChild) {
                    state.initialTransition = {
                        event: '@Q_INIT_SIG',
                        target: initialChild.name
                    };
                } else if (state.children.length > 0) {
                    // Default to first child
                    state.initialTransition = {
                        event: '@Q_INIT_SIG',
                        target: state.children[0].name
                    };
                }
            }
        }
    }
}