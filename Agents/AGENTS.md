# AGENTS.md

## Project Startup Rule

For every new project startup, the agent must refresh working memory, reload project context, and raise execution readiness before making changes.

For this MotionBERT project, startup execution must include all of the following:

1. Read the current agent framework documents in `Agents/` before modifying code or docs:
   - `Agents/AGENTS.md`
   - `Agents/docs_REQUIREMENT_WORKFLOW.md`
   - `Agents/docs_MAINTENANCE_PROGRESS.md`
2. Rebuild project context from the current workspace state instead of relying on previous conversation memory.
3. Check the project's runnable entry points and deployment docs first:
   - `docs/local_deploy_windows.md`
   - `run_alphapose.bat`
   - `run_motionbert_pose.bat`
   - `run_motionbert_mesh.bat`
   - `run_motionbert_realtime.bat`
4. Verify whether the task affects:
   - AlphaPose inference chain
   - MotionBERT 3D pose inference chain
   - Mesh inference chain
   - Realtime camera / stream inference chain
   - Local Windows deployment scripts or docs
5. Before editing, identify the current source of truth for:
   - environments under `.local/miniconda3/envs`
   - external dependency checkout under `.external/AlphaPose`
   - current runtime outputs under `outputs/`
   - deployment and usage documentation under `docs/`
6. If the request is non-trivial, explicitly decompose the work and maintain progress records while executing.

## Requirement Clarification Rule

For every new requirement from the project owner:

- If any acceptance criteria, scope boundary, or data behavior is unclear, the agent must ask concise follow-up questions first.
- The agent must not silently assume ambiguous business rules in high-impact changes.
- Clarification must cover: expected behavior, exception cases, priority, and definition of done.

## Delivery Process Rule (Professional Agent Team)

All non-trivial requests must follow an agent-team scientific workflow:

1. Requirement framing and ambiguity scan.
2. Clarification questions if needed.
3. Task decomposition across code, data, docs, scripts, and QA.
4. Delegate to specialized sub-agents in parallel where appropriate.
5. Integrate, verify, and regression test.
6. Update documentation and maintenance progress records.

## Tacit Knowledge Rule (Michael Polanyi)

When understanding requirements and planning solutions, apply tacit knowledge:

- Combine explicit requirements with contextual signals from historical decisions, user habits, and product constraints.
- Make implicit assumptions explicit in the plan or questions.
- Prefer practical, experience-based tradeoffs that reduce delivery risk.

## Performance Priority Rule

When the project owner explicitly prioritizes runtime quality, responsiveness, smoothness, latency, tracking stability, or overall effect over resource savings, the agent must optimize for the best achievable result on the available machine before optimizing for efficiency.

- Do not prematurely conserve CPU, GPU, RAM, VRAM, bandwidth, or disk IO when the owner has stated that higher resource usage is acceptable.
- In this mode, the default priority order is:
  1. user-visible quality and stability
  2. end-to-end latency and responsiveness
  3. throughput / frame rate
  4. resource efficiency
- The agent should actively use available compute budget if doing so improves the target experience in a meaningful way.
- Only back off to lower-resource configurations when:
  - the higher-resource path is unstable,
  - it causes regressions worse than the benefit,
  - it blocks delivery,
  - or the owner asks to rebalance toward efficiency.
- For real-time pipelines, this rule applies across the full chain:
  - capture
  - preprocessing
  - model inference
  - transport
  - engine-side consumption
  - interpolation / buffering

## Mature Solution First Rule

When solving a performance, latency, transport, mocap, retargeting, or streaming problem, the agent must check for mature existing solutions before hand-optimizing custom code.

- Prioritize:
  - official engine capabilities
  - official plugins
  - established open-source plugins
  - widely adopted maintained projects
  - documented production patterns
- If a mature solution can solve the problem faster or better than custom iteration, evaluate and prefer it first.
- If the agent still proceeds with custom optimization, it should be because:
  - the existing solution cannot fit the current MotionBERT path,
  - the integration cost is too high,
  - the solution is abandoned or too limited,
  - or the current code can be improved faster with lower delivery risk.

## External Capability First Rule

Before implementing any non-trivial feature, integration, automation, or workflow, the agent must search the internet first to check whether there is an existing plugin, SDK, client library, API, service, or official integration that can be used directly.

- Do not default to handwritten custom code when a mature existing capability can solve the requirement faster, more reliably, or with less maintenance risk.
- Prioritize official vendor APIs, official SDKs, official plugins, and widely adopted maintained libraries over ad hoc self-built integrations.
- If an existing capability is found, evaluate it first on fit, maintenance cost, performance, licensing, deployment complexity, and lock-in risk before deciding to build custom code.
- If the agent still chooses custom implementation, it must be because the searched options are unavailable, unfit, too limited, unsafe, abandoned, incompatible with the project, or create higher delivery risk.
- For high-impact external capabilities, record the key search conclusion in the delivery summary or related project documentation so later work does not repeat the same evaluation.

## Unreal Engine Expert Mode

When the requirement involves Unreal Engine, MetaHuman, animation retargeting, mocap streaming, editor automation, virtual production, or runtime data transport into a UE project, the agent must switch into a UE-focused execution mode.

- First inspect whether the workspace contains a UE project such as a `.uproject`, `Config/`, `Content/`, `Plugins/`, `Source/`, or related build files.
- Before writing custom transport code, search for official Unreal capabilities first, especially:
  - Live Link
  - Live Link Hub
  - official UE plugins
  - official Blueprint-accessible runtime plugins
  - official Python or C++ APIs
- For MotionBERT to UE data flow, prefer a direct path that minimizes custom protocol design and custom engine-side code.
- Preferred evaluation order for mocap-style streaming is:
  1. official Live Link based path
  2. official runtime plugins such as OSC or UDP-accessible UE capabilities
  3. existing maintained third-party UE plugins
  4. custom transport or custom UE plugin implementation
- The agent must distinguish clearly between:
  - editor-only automation
  - runtime in-editor preview
  - packaged runtime behavior
  - animation retargeting / skeleton mapping
  - transport layer and data schema
- If the target is to drive a UE skeletal mesh, the agent must explicitly inspect and document:
  - source skeleton naming
  - target skeleton naming
  - retarget or remap strategy
  - coordinate system conversion
  - frame rate, latency, and buffering strategy
  - whether the path depends on editor-only features
- When a UE project is present, any implementation proposal must include the exact insertion points in that UE project, such as plugin, Blueprint, animation blueprint, C++ module, or utility script location.

## Maintenance Progress Rule

After each implemented request, update:

- `Agents/docs_MAINTENANCE_PROGRESS.md` with date, scope, status, and next actions, using the current project progress template structure.
- Related design, operation, deployment, or test documents when behavior or process changes.
- During active testing, update maintenance progress and high-level test records at least once for about every three meaningful test steps, or earlier when a key finding appears.
- During active testing, maintain a separate step-by-step test execution log document and update it immediately after each formal test step.
- For `Agents/docs_MAINTENANCE_PROGRESS.md`, append new progress entries only after the current real project baseline is established; example placeholder content must not be retained.
- Every maintenance progress entry must use a timestamp accurate to the second.
- Keep test documents structurally consistent: section naming, status grouping, and ordering must follow the same pattern across updates.
- In test records and checklists, preserve strict sequence order; do not leave missing step numbers or append later-numbered items ahead of earlier-numbered items.
