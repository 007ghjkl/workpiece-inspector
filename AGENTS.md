# Project AI Operating Rules

## Core Principle

The AI agent is not a product owner.

The AI agent must:

1. Discover requirements.
2. Document assumptions.
3. Present tradeoffs.
4. Implement approved specifications.

The AI agent must NOT:

1. Invent business requirements.
2. Change architecture without approval.
3. Add features not defined in specs.
4. Ignore uncertainty.

Whenever uncertainty exists:

- Stop.
- Explain uncertainty.
- Propose options.
- Wait for confirmation.

---

# Development Workflow

Every task must follow this order.

Research
→ PRD
→ Architecture
→ Tasks
→ Spec
→ Implementation
→ Test
→ Review

Do not skip stages.

Do not write production code before:

- PRD exists
- Architecture exists
- Task exists

---

# Source Of Truth

Priority order:

1. Current Task
2. Technical Spec
3. Architecture Document
4. PRD
5. Research Documents

If documents conflict:

- Report conflict.
- Do not guess.

---

# Project Structure

docs/

    prd.md
    architecture.md

research/

specs/

tasks/

src/

tests/

---

# Research Rules

When performing research:

1. Identify existing industry solutions.
2. Identify open-source references.
3. Identify common architectures.
4. Identify known risks.
5. Identify alternative approaches.

Output:

research/<topic>.md

Never start implementation during research.

---

# PRD Rules

PRD must contain:

- Problem statement
- User roles
- User stories
- Functional requirements
- Non-functional requirements
- MVP scope
- Out-of-scope items

Output:

docs/prd.md

No implementation details allowed.

---

# Architecture Rules

Architecture must include:

- System overview
- Components
- Data flow
- APIs
- Database design
- Deployment model
- Scalability concerns
- Security concerns

Output:

docs/architecture.md

Before implementation:

Generate at least two architecture options.

Explain tradeoffs.

Recommend one.

---

# Task Planning Rules

Tasks must be:

- Small
- Independent
- Testable

Maximum task size:

1–3 hours of implementation effort.

Task format:

## Goal

## Scope

## Dependencies

## Acceptance Criteria

## Test Requirements

Output:

tasks/XXX.md

---

# Spec Rules

Every feature requires a spec.

Spec must define:

- Inputs
- Outputs
- APIs
- Data structures
- Error handling
- Edge cases

Implementation must follow spec.

---

# Coding Rules

Implement only the current task.

Do not modify unrelated files.

Minimize code changes.

Follow existing project patterns.

Avoid premature abstraction.

Avoid introducing new frameworks without approval.

---

# Testing Rules

Every implementation must include:

- Unit tests
- Integration tests when applicable

Before task completion:

- Run tests
- Verify acceptance criteria

If tests cannot be executed:

Explain why.

---

# Review Rules

After implementation:

Review:

1. Correctness
2. Simplicity
3. Security
4. Performance
5. Maintainability

Output:

review_report.md

Include:

- Risks
- Technical debt
- Future improvements

---

# Documentation Rules

Whenever code changes:

Update relevant:

- Specs
- Architecture
- README

Documentation is part of the task.

Task is not complete until documentation is updated.

---

# Decision Log

Any significant decision must be recorded.

Location:

docs/decisions/

Format:

YYYY-MM-DD-topic.md

Include:

- Context
- Options
- Decision
- Consequences

---

# Security Rules

Never:

- Hardcode secrets
- Commit credentials
- Disable authentication
- Ignore validation

Always:

- Validate inputs
- Handle errors
- Follow least privilege

---

# Communication Rules

When blocked:

Do not guess.

Provide:

1. Problem
2. Evidence
3. Possible solutions
4. Recommendation

Then stop.

---

# Completion Criteria

A task is complete only if:

- Code implemented
- Tests added
- Tests pass
- Documentation updated
- Review completed

---

# Anti-Hallucination Rules

Never assume an API exists.

Never assume a database schema exists.

Never assume a third-party library behavior.

Always inspect the codebase first.

Before modifying code:

1. Find relevant files.
2. Read relevant files.
3. Explain understanding.
4. Then modify.

If evidence is missing:

Stop and ask.
