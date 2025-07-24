import { StateMachine, State, Transition } from '../types/stateMachine';

export interface ValidationError {
    severity: 'error' | 'warning' | 'info';
    message: string;
    location?: {
        state?: string;
        transition?: string;
        line?: number;
    };
}

/**
 * Validates QP state machines for correctness and best practices
 */
export class StateMachineValidator {
    private errors: ValidationError[] = [];
    private visitedStates: Set<string> = new Set();

    validate(stateMachine: StateMachine): ValidationError[] {
        this.errors = [];
        this.visitedStates.clear();

        // Basic validations
        this.validateBasicStructure(stateMachine);
        
        // Validate state hierarchy
        this.validateStates(stateMachine.states);
        
        // Check for unreachable states
        this.checkReachability(stateMachine);
        
        // Validate transitions
        this.validateTransitions(stateMachine);
        
        // QP-specific validations
        this.validateQPConventions(stateMachine);

        return this.errors;
    }

    private validateBasicStructure(stateMachine: StateMachine) {
        // Must have a name
        if (!stateMachine.name || stateMachine.name === 'StateMachine') {
            this.addError('warning', 'State machine should have a meaningful name');
        }

        // Must have an initial state
        if (!stateMachine.initialState) {
            this.addError('error', 'State machine must have an initial state');
        }

        // Must have at least one state
        if (stateMachine.states.length === 0) {
            this.addError('error', 'State machine must have at least one state');
        }

        // Check for duplicate state names
        const stateNames = new Set<string>();
        this.collectStateNames(stateMachine.states, stateNames);
    }

    private collectStateNames(states: State[], names: Set<string>, prefix: string = '') {
        for (const state of states) {
            const fullName = prefix ? `${prefix}.${state.name}` : state.name;
            
            if (names.has(state.name)) {
                this.addError('error', `Duplicate state name: ${state.name}`, { state: fullName });
            }
            names.add(state.name);

            if (state.children.length > 0) {
                this.collectStateNames(state.children, names, fullName);
            }
        }
    }

    private validateStates(states: State[], parentPath: string = '') {
        for (const state of states) {
            const statePath = parentPath ? `${parentPath}.${state.name}` : state.name;

            // Validate state name
            if (!this.isValidStateName(state.name)) {
                this.addError('warning', 
                    `State name '${state.name}' should start with uppercase and use CamelCase`,
                    { state: statePath });
            }

            // Composite states must have initial transition
            if (state.children.length > 0 && !state.initialTransition) {
                this.addError('error', 
                    `Composite state '${state.name}' must have an initial transition`,
                    { state: statePath });
            }

            // Check for empty composite states
            if (state.children.length === 0 && state.initialTransition) {
                this.addError('warning', 
                    `State '${state.name}' has initial transition but no child states`,
                    { state: statePath });
            }

            // Validate entry/exit actions
            this.validateActions(state, statePath);

            // Recurse into children
            if (state.children.length > 0) {
                this.validateStates(state.children, statePath);
            }
        }
    }

    private validateActions(state: State, statePath: string) {
        // Check for common mistakes in actions
        const allActions = [...state.entryActions, ...state.exitActions];
        
        for (const action of allActions) {
            // Check for missing semicolons (common in QP)
            if (!action.endsWith(')') && !action.endsWith(';')) {
                this.addError('warning', 
                    `Action '${action}' should end with semicolon`,
                    { state: statePath });
            }

            // Check for undefined me-> references
            if (action.includes('me->') && !action.includes('&me->')) {
                const memberMatch = action.match(/me->(\w+)/);
                if (memberMatch) {
                    // TODO: Check if member exists in data members
                }
            }
        }
    }

    private checkReachability(stateMachine: StateMachine) {
        // Start from initial state and traverse all reachable states
        this.visitedStates.clear();
        
        if (stateMachine.initialState) {
            this.traverseFromState(stateMachine.initialState, stateMachine.states);
        }

        // Check for unreachable states
        this.checkUnreachableStates(stateMachine.states);
    }

    private traverseFromState(stateName: string, allStates: State[]) {
        if (this.visitedStates.has(stateName)) {
            return;
        }

        this.visitedStates.add(stateName);
        const state = this.findState(stateName, allStates);
        
        if (!state) {
            return;
        }

        // Visit all transition targets
        for (const transition of state.transitions) {
            if (!transition.isInternal) {
                this.traverseFromState(transition.target, allStates);
            }
        }

        // Visit initial state of composite states
        if (state.initialTransition) {
            this.traverseFromState(state.initialTransition.target, allStates);
        }
    }

    private checkUnreachableStates(states: State[], parentPath: string = '') {
        for (const state of states) {
            const statePath = parentPath ? `${parentPath}.${state.name}` : state.name;
            
            if (!this.visitedStates.has(state.name) && state.name !== '[*]') {
                this.addError('warning', 
                    `State '${state.name}' is unreachable`,
                    { state: statePath });
            }

            if (state.children.length > 0) {
                this.checkUnreachableStates(state.children, statePath);
            }
        }
    }

    private validateTransitions(stateMachine: StateMachine) {
        this.validateStateTransitions(stateMachine.states);
    }

    private validateStateTransitions(states: State[], parentPath: string = '') {
        for (const state of states) {
            const statePath = parentPath ? `${parentPath}.${state.name}` : state.name;

            for (const transition of state.transitions) {
                // Check target exists
                if (!this.isSpecialTarget(transition.target)) {
                    const targetExists = this.findState(transition.target, states);
                    if (!targetExists) {
                        this.addError('error', 
                            `Transition target '${transition.target}' does not exist`,
                            { state: statePath, transition: transition.event });
                    }
                }

                // Validate guard conditions
                if (transition.guard) {
                    this.validateGuard(transition.guard, statePath, transition.event);
                }

                // Check for duplicate transitions
                const duplicates = state.transitions.filter(t => 
                    t.event === transition.event && 
                    t.guard === transition.guard &&
                    t !== transition
                );
                
                if (duplicates.length > 0) {
                    this.addError('error', 
                        `Duplicate transition for event '${transition.event}'`,
                        { state: statePath });
                }
            }

            if (state.children.length > 0) {
                this.validateStateTransitions(state.children, statePath);
            }
        }
    }

    private validateGuard(guard: string, statePath: string, event: string) {
        // Check for common syntax errors
        const openParens = (guard.match(/\(/g) || []).length;
        const closeParens = (guard.match(/\)/g) || []).length;
        
        if (openParens !== closeParens) {
            this.addError('error', 
                `Unbalanced parentheses in guard: ${guard}`,
                { state: statePath, transition: event });
        }

        // Check for common mistakes
        if (guard.includes('=') && !guard.includes('==') && !guard.includes('<=') && !guard.includes('>=')) {
            this.addError('warning', 
                `Possible assignment instead of comparison in guard: ${guard}`,
                { state: statePath, transition: event });
        }
    }

    private validateQPConventions(stateMachine: StateMachine) {
        // Check event naming conventions
        for (const event of stateMachine.events) {
            if (!this.isValidEventName(event.name)) {
                this.addError('warning', 
                    `Event '${event.name}' should be in UPPER_CASE format`);
            }

            // Check event values
            if (!event.value.includes('Q_USER_SIG')) {
                this.addError('warning', 
                    `Event '${event.name}' should be based on Q_USER_SIG`);
            }
        }

        // Check for QTimeEvt in data members if timer events are used
        const hasTimeoutEvents = this.hasTimeoutEvents(stateMachine.states);
        const hasTimerMember = stateMachine.dataMembers.some(m => m.type === 'QTimeEvt');
        
        if (hasTimeoutEvents && !hasTimerMember) {
            this.addError('warning', 
                'State machine uses TIMEOUT events but has no QTimeEvt data member');
        }
    }

    private hasTimeoutEvents(states: State[]): boolean {
        for (const state of states) {
            if (state.transitions.some(t => t.event === 'TIMEOUT')) {
                return true;
            }
            if (state.children.length > 0 && this.hasTimeoutEvents(state.children)) {
                return true;
            }
        }
        return false;
    }

    private findState(name: string, states: State[]): State | undefined {
        for (const state of states) {
            if (state.name === name) {
                return state;
            }
            const found = this.findState(name, state.children);
            if (found) {
                return found;
            }
        }
        return undefined;
    }

    private isSpecialTarget(target: string): boolean {
        return target === '[*]' || target.startsWith('[H') || target.includes('History');
    }

    private isValidStateName(name: string): boolean {
        return /^[A-Z][a-zA-Z0-9]*$/.test(name);
    }

    private isValidEventName(name: string): boolean {
        return /^[A-Z][A-Z0-9_]*$/.test(name);
    }

    private addError(severity: ValidationError['severity'], message: string, location?: ValidationError['location']) {
        this.errors.push({ severity, message, location });
    }
}