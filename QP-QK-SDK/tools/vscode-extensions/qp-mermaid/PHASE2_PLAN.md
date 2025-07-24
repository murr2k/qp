# Phase 2: Enhanced QP Mermaid Extension

## Overview

Phase 2 focuses on bridging the gap between basic Mermaid diagrams and full QP state machine capabilities, creating a truly powerful visual development environment for embedded systems.

## Goals

1. **Full QP Syntax Support** - Handle all QP-specific constructs while maintaining visual preview
2. **Hierarchical State Machines** - Support composite states and proper state nesting
3. **Enhanced Code Generation** - Generate production-ready QP code
4. **Improved Developer Experience** - Better validation, IntelliSense, and debugging

## Technical Approach

### 1. Mermaid Preprocessor (Week 1)

Create a preprocessor that converts QP-enhanced syntax to standard Mermaid for visualization while preserving the original for code generation.

```typescript
// Before preprocessing (QP syntax)
state Operational {
    entry: system_init()
    exit: system_cleanup()
    
    state Active {
        entry: activate_sensors()
        [*] --> Idle
    }
}

// After preprocessing (Mermaid compatible)
state Operational {
    Active --> Idle : [initial]
}
%% QP-META: {"Operational": {"entry": ["system_init()"], "exit": ["system_cleanup()"]}}
```

### 2. Hierarchical State Machine Support (Week 1-2)

Implement proper composite state handling:
- Initial transitions within states
- State nesting (multiple levels)
- Proper scope resolution for transitions
- Parent state entry/exit handling

### 3. Enhanced Parser Architecture (Week 2)

```typescript
interface QPStateMachine extends StateMachine {
    // Phase 1 properties plus:
    history: Map<string, HistoryType>;
    regions: OrthogonalRegion[];
    includes: string[]; // External files
    namespace?: string;
}

interface CompositeState extends State {
    initialState?: string;
    historyType?: 'shallow' | 'deep';
    regions?: State[][]; // Orthogonal regions
}
```

### 4. Advanced Transitions (Week 2)

Support for:
- Complex guard expressions: `[count > 5 && isReady()]`
- Multiple actions: `EVENT / action1(); action2()`
- Choice pseudostates
- Fork/join for concurrent states

### 5. Custom Visual Editor (Week 3-4)

Build a custom webview with:
- D3.js or Cytoscape.js for rendering
- Drag-and-drop state creation
- Direct manipulation of transitions
- Property panels for states/transitions
- Live code preview side-by-side

### 6. Semantic Validation (Week 3)

Implement QP-specific validations:
- Unreachable states detection
- Missing initial transitions
- Guard condition validation
- Event usage analysis
- Naming convention checks

### 7. Testing Framework (Week 4)

- Unit tests for parser and generator
- Integration tests with real QP projects
- Visual regression tests for diagrams
- Performance benchmarks

## Implementation Phases

### Phase 2.1: Core Enhancements (Weeks 1-2)
- [ ] Mermaid preprocessor
- [ ] Hierarchical state support
- [ ] Enhanced parser
- [ ] Guard conditions
- [ ] History states

### Phase 2.2: Visual Editor (Weeks 3-4)
- [ ] Custom webview implementation
- [ ] State manipulation UI
- [ ] Property editors
- [ ] Live code preview
- [ ] Undo/redo support

### Phase 2.3: Advanced Features (Week 5)
- [ ] QM import/export
- [ ] Multi-file support
- [ ] Code templates customization
- [ ] Debugging integration

### Phase 2.4: Polish & Release (Week 6)
- [ ] Performance optimization
- [ ] Documentation
- [ ] Tutorial videos
- [ ] VS Code Marketplace submission

## Example: Complex State Machine

```mermaid
%%{init: {
    'qp': {
        'activeObject': 'Calculator',
        'includes': ['calculator_events.h'],
        'dataMembers': [
            {type: 'double', name: 'operand1'},
            {type: 'double', name: 'operand2'},
            {type: 'char', name: 'operator'}
        ]
    }
}}%%

stateDiagram-v2
    [*] --> Off
    
    state On {
        entry: display_clear()
        exit: display_off()
        
        [*] --> Ready
        
        state Ready {
            entry: display_show(0)
            
            Ready --> Operand1 : DIGIT / store_digit()
            Ready --> Operand1 : POINT / begin_decimal()
        }
        
        state Operand1 {
            entry: display_show(me->operand1)
            
            Operand1 --> Operand1 : DIGIT / append_digit()
            Operand1 --> Operand1 : POINT [!has_decimal()] / begin_decimal()
            Operand1 --> OpEntered : OPER / store_operator()
        }
        
        state OpEntered {
            entry: display_show_op(me->operator)
            
            OpEntered --> Operand2 : DIGIT / store_digit()
            OpEntered --> Result : OPER [is_unary()] / calculate()
        }
        
        state Operand2 {
            entry: display_show(me->operand2)
            
            Operand2 --> Operand2 : DIGIT / append_digit()
            Operand2 --> Result : EQUALS / calculate()
            Operand2 --> Result : OPER / calculate(); store_operator()
        }
        
        state Result {
            entry: display_result(me->result)
            
            Result --> Ready : CE
            Result --> Operand1 : DIGIT / store_digit()
            Result --> OpEntered : OPER / store_operator()
        }
        
        Ready --> Off : OFF
        Operand1 --> Off : OFF  
        OpEntered --> Off : OFF
        Operand2 --> Off : OFF
        Result --> Off : OFF
        
        --[H*]--> Ready : CE
    }
    
    Off --> On : ON
```

## Success Metrics

1. **Feature Completeness**: Support 90% of QM features
2. **Performance**: Handle state machines with 100+ states
3. **Adoption**: 5000+ downloads in first 3 months
4. **Quality**: Zero critical bugs, <5ms diagram updates
5. **Documentation**: 100% API coverage, 10+ examples

## Next Steps

1. Start with Mermaid preprocessor (highest impact)
2. Implement hierarchical states (core QP feature)
3. Build incremental visual enhancements
4. Gather community feedback early and often

The goal is to make QP development as intuitive as drawing a diagram while maintaining the framework's power and correctness.