import { State, StateMachine, Transition } from '../types/stateMachine';

/**
 * Enhanced parser for hierarchical state machines
 */
export class HierarchicalStateParser {
    private stateMap: Map<string, State> = new Map();
    private currentPath: string[] = [];

    /**
     * Parses hierarchical state structures and resolves scoping
     */
    parseHierarchicalStates(stateMachine: StateMachine): StateMachine {
        // Build state map for quick lookup
        this.buildStateMap(stateMachine.states);
        
        // Resolve transition targets with proper scoping
        this.resolveTransitions(stateMachine.states);
        
        // Set up initial transitions for composite states
        this.setupInitialTransitions(stateMachine.states);
        
        return stateMachine;
    }

    private buildStateMap(states: State[], parentPath: string = '') {
        for (const state of states) {
            const fullPath = parentPath ? `${parentPath}.${state.name}` : state.name;
            this.stateMap.set(fullPath, state);
            
            // Also store by simple name for local resolution
            if (!this.stateMap.has(state.name)) {
                this.stateMap.set(state.name, state);
            }
            
            if (state.children.length > 0) {
                this.buildStateMap(state.children, fullPath);
            }
        }
    }

    private resolveTransitions(states: State[], parentPath: string = '') {
        for (const state of states) {
            const currentPath = parentPath ? `${parentPath}.${state.name}` : state.name;
            
            // Resolve transitions for this state
            for (const transition of state.transitions) {
                transition.target = this.resolveTarget(transition.target, currentPath);
            }
            
            // Resolve initial transition if present
            if (state.initialTransition) {
                state.initialTransition.target = this.resolveTarget(
                    state.initialTransition.target, 
                    currentPath
                );
            }
            
            // Recurse into children
            if (state.children.length > 0) {
                this.resolveTransitions(state.children, currentPath);
            }
        }
    }

    private resolveTarget(target: string, fromPath: string): string {
        // Handle special targets
        if (target === '[*]' || target.startsWith('[H')) {
            return target;
        }
        
        // Try absolute path first
        if (this.stateMap.has(target)) {
            return target;
        }
        
        // Try relative to current state's parent
        const pathParts = fromPath.split('.');
        for (let i = pathParts.length; i >= 0; i--) {
            const basePath = pathParts.slice(0, i).join('.');
            const candidatePath = basePath ? `${basePath}.${target}` : target;
            
            if (this.stateMap.has(candidatePath)) {
                return candidatePath;
            }
        }
        
        // Try siblings
        if (pathParts.length > 1) {
            const parentPath = pathParts.slice(0, -1).join('.');
            const siblingPath = `${parentPath}.${target}`;
            if (this.stateMap.has(siblingPath)) {
                return siblingPath;
            }
        }
        
        // Return as-is if not found (will be caught by validation)
        return target;
    }

    private setupInitialTransitions(states: State[]) {
        for (const state of states) {
            if (state.children.length > 0 && !state.initialTransition) {
                // Look for explicit initial transition
                const initialChild = state.children.find(child => child.isInitial);
                
                if (initialChild) {
                    state.initialTransition = {
                        event: '@Q_INIT_SIG',
                        target: initialChild.name
                    };
                } else {
                    // Default to first child
                    state.initialTransition = {
                        event: '@Q_INIT_SIG',
                        target: state.children[0].name
                    };
                }
            }
            
            // Recurse into children
            if (state.children.length > 0) {
                this.setupInitialTransitions(state.children);
            }
        }
    }

    /**
     * Finds the Least Common Ancestor (LCA) of two states
     * Used for determining transition paths in hierarchical state machines
     */
    findLCA(state1Path: string, state2Path: string): string {
        const path1Parts = state1Path.split('.');
        const path2Parts = state2Path.split('.');
        
        let commonPath = '';
        for (let i = 0; i < Math.min(path1Parts.length, path2Parts.length); i++) {
            if (path1Parts[i] === path2Parts[i]) {
                commonPath = commonPath ? `${commonPath}.${path1Parts[i]}` : path1Parts[i];
            } else {
                break;
            }
        }
        
        return commonPath;
    }

    /**
     * Calculates entry and exit sequences for transitions
     */
    calculateTransitionSequence(fromPath: string, toPath: string): {
        exitSequence: string[];
        entrySequence: string[];
        lca: string;
    } {
        const lca = this.findLCA(fromPath, toPath);
        
        // Calculate exit sequence (from source to LCA)
        const exitSequence: string[] = [];
        let currentPath = fromPath;
        while (currentPath && currentPath !== lca) {
            exitSequence.push(currentPath);
            const parts = currentPath.split('.');
            parts.pop();
            currentPath = parts.join('.');
        }
        
        // Calculate entry sequence (from LCA to target)
        const entrySequence: string[] = [];
        const targetParts = toPath.split('.');
        const lcaParts = lca ? lca.split('.') : [];
        
        for (let i = lcaParts.length; i < targetParts.length; i++) {
            const path = targetParts.slice(0, i + 1).join('.');
            entrySequence.push(path);
        }
        
        return { exitSequence, entrySequence, lca };
    }
}