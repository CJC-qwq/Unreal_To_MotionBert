# docs_REQUIREMENT_WORKFLOW.md

# Requirement To Delivery Workflow

## 1. Goal

Convert business or product requirements into verifiable implementation results with minimal ambiguity, rework, and hidden delivery risk.

## 2. Execution Principles

- If the requirement is unclear, ask first. Do not make high-risk guesses.
- Any task larger than a single-point patch should use a professional agent-team style breakdown.
- Before writing custom integration or workflow code, search the internet first for existing plugins, SDKs, client libraries, official APIs, or maintained third-party solutions that can be used directly.
- Prefer existing external capabilities when they materially reduce implementation effort, maintenance burden, or delivery risk.
- After every delivery, update the maintenance progress document.
- During planning, apply Michael Polanyi style tacit knowledge: combine explicit requests with context, historical decisions, real usage patterns, and hidden constraints.

## 3. Standard Workflow

### 3.1 Requirement Intake And Ambiguity Scan

- Identify goal, scope, acceptance criteria, boundary conditions, and exception paths.
- Mark every point that cannot be safely assumed.

### 3.2 Clarification

- Ask the minimum necessary follow-up questions.
- Produce an executable Definition of Done.

### 3.3 Agent Team Breakdown

- Product or process agent: business behavior and workflow rules
- Backend or script agent: data flow, interface behavior, automation, exception handling
- Frontend or interaction agent: UI state, interaction feedback, usability
- QA agent: verification paths, regression checklist, retest coverage

### 3.4 Scientific Implementation

- Build the smallest verifiable closed loop first, then iterate.
- Every step must produce an observable result such as logs, script output, page state, or test evidence.

### 3.5 Verification And Regression

- Run the required build, script, inference, or test steps.
- Verify the main path and the important failure paths.
- Add regression checks when the change risk is meaningful.

### 3.6 Documentation And Progress Update

- Update related design, deployment, runtime, or test documents.
- Append a maintenance progress record.

## 4. Output Template

- Requirement summary
- Key clarification points
- Implementation checklist by module
- Risk and rollback strategy
- Verification results
- Next-step plan
