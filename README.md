# Quantum Leaps (QP) Framework Study

This repository contains comprehensive research and reference materials for understanding and using the Quantum Leaps (QP) framework by Miro Samek for microcontroller firmware development.

## Overview

The Quantum Platform (QP) is a family of open-source real-time embedded frameworks (RTEFs) based on the Active Object (Actor) design pattern. QP enables modern event-driven programming with hierarchical state machines in deeply embedded systems.

## Repository Contents

- **[QP_Microcontroller_Reference.md](QP_Microcontroller_Reference.md)** - Quick reference guide with code examples, patterns, and best practices for QP development
- **[QP_Getting_Started_Guide.md](QP_Getting_Started_Guide.md)** - Comprehensive getting started guide covering installation, setup, and first projects

## Key Concepts

### Active Objects
Event-driven, encapsulated objects that communicate asynchronously through message passing, eliminating shared-state concurrency issues.

### Hierarchical State Machines (HSMs)
UML-based state machines with behavioral inheritance, enabling complex behavior modeling with clear structure.

### Built-in Kernels
- **QV** - Cooperative kernel (simplest, recommended default)
- **QK** - Preemptive non-blocking kernel (hard real-time)
- **QXK** - Dual-mode kernel (supports blocking operations)

## Getting Started

1. Download QP bundle from [QuantumLeaps GitHub](https://github.com/QuantumLeaps)
2. Install QM modeling tool for visual state machine design
3. Review the reference materials in this repository
4. Start with the Blinky example for your target platform
5. Progress to more complex examples like DPP (Dining Philosophers)

## Resources

- [Official Website](https://www.state-machine.com)
- [GitHub Organization](https://github.com/QuantumLeaps)
- [Modern Embedded Systems Programming Course](https://www.youtube.com/playlist?list=PLPW8O6W-1chwyTzI3BHwBLbGQoPFxPAPM)
- [Book: Practical UML Statecharts in C/C++](https://www.state-machine.com/psicc2)

## License

This repository contains educational materials about the Quantum Leaps framework. The QP framework itself is dual-licensed under GPL and commercial licenses. See the official Quantum Leaps documentation for licensing details.