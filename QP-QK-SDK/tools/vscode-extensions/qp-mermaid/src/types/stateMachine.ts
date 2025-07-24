export interface StateMachine {
    name: string;
    type: 'QActive' | 'QMsm' | 'QHsm';
    priority?: number;
    stackSize?: number;
    events: Event[];
    states: State[];
    initialState: string;
    dataMembers: DataMember[];
}

export interface Event {
    name: string;
    value: string;
    parameters?: Parameter[];
}

export interface State {
    name: string;
    parent?: string;
    entryActions: string[];
    exitActions: string[];
    transitions: Transition[];
    children: State[];
    isInitial?: boolean;
    initialTransition?: Transition;
}

export interface Transition {
    event: string;
    target: string;
    guard?: string;
    action?: string;
    isInternal?: boolean;
}

export interface DataMember {
    type: string;
    name: string;
    comment?: string;
    initialValue?: string;
}

export interface Parameter {
    type: string;
    name: string;
}

export interface GeneratorOptions {
    language: 'C' | 'C++';
    includeComments: boolean;
}

export interface GeneratedCode {
    header: string;
    source: string;
    interface?: string;
}