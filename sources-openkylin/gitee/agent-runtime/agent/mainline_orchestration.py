"""Stable system guidance for mainline-first task execution."""

TASK_EXECUTION_GUIDANCE = (
    "<task_execution>\n"
    "The main agent is the default executor. Complete the task continuously in "
    "the current context and do not proactively create subagents merely to "
    "reduce context length. For non-trivial work, use `todo` when an explicit "
    "plan materially improves correctness, and keep its statuses current.\n"
    "\n"
    "Use `delegate_task` only when the user explicitly requests delegation, "
    "subagents, or a mainline/branch workflow. Generic complexity, multiple "
    "steps, or possible parallelism alone is not permission to delegate. When "
    "delegation is explicitly requested, pass the necessary context and verify "
    "child results in the main agent.\n"
    "\n"
    "Execute any plan to completion rather than stopping after planning:\n"
    "- Respect dependencies and list order; finish and verify the current item "
    "before starting a dependent item.\n"
    "- After a branch returns, update TODO with its result before promoting or "
    "dispatching another item.\n"
    "- After each item finishes, immediately update `todo`: mark it completed, "
    "promote the next item, and continue until no pending/in-progress items "
    "remain.\n"
    "- Revise, add, reorder, or cancel TODO items when execution reveals that "
    "the original plan is incomplete or wrong.\n"
    "- If indispensable user input or approval is missing, use `clarify`. Keep "
    "the plan intact while waiting; after the answer, resume it or update it "
    "before continuing.\n"
    "- Do not claim completion while required TODO items remain pending or "
    "in_progress. A final answer is accepted only after every required TODO is "
    "completed or explicitly cancelled as no longer required. Verify the final "
    "result against the user's request.\n"
    "</task_execution>"
)

# Compatibility for extensions importing the former symbol. The guidance is
# intentionally mainline-first and no longer enables proactive branching.
MAINLINE_ORCHESTRATION_GUIDANCE = TASK_EXECUTION_GUIDANCE
