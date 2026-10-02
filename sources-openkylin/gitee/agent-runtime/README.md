# Kylin Agent Runtime


**Kylin Agent Runtime is a self-improving agent runtime built on Hermes.** It extends Hermes with improved trunk and branch collaboration methods, stronger memory, self-evolution constraints, more detailed process visibility, and ongoing refinements to the agent experience. It has a built-in learning loop — it creates skills from experience, improves them during use, nudges itself to persist knowledge, searches its own past conversations, and builds a deepening model of who you are across sessions. 

## Installation

Use `uv` to create a virtual environment and install this project in editable mode.

```bash
git clone https://gitee.com/openkylin/agent-runtime.git
cd agent-runtime

curl -LsSf https://astral.sh/uv/install.sh | sh
uv venv .venv --python 3.11
source .venv/bin/activate
uv pip install -e ".[all]"
```

If you already have `uv` and Python 3.11 installed, you can start from the virtual environment step.

## Running Agent Runtime

Before the first run, activate the virtual environment and configure your model provider, API key, and other basic settings:

```bash
source .venv/bin/activate
kylin-agent-runtime setup
```

After setup, start the interactive agent from the project directory:

```bash
kylin-agent-runtime
```

Other available run modes include:

```bash
# Launch the modern terminal UI (TUI)
kylin-agent-runtime --tui

# Run a one-shot task and print only the final result
kylin-agent-runtime -z "Analyze the current project and suggest improvements"

# Start the web dashboard (port 9119 by default)
kylin-agent-runtime dashboard
```

### Running the Messaging Gateway

On Linux, the gateway uses the `kylin-agent-runtime-gateway.service` systemd user unit. You can also manage it directly with `systemctl`:

```bash
# Start, stop, or restart the service
systemctl --user start kylin-agent-runtime-gateway.service
systemctl --user stop kylin-agent-runtime-gateway.service
systemctl --user restart kylin-agent-runtime-gateway.service

# Check service status
systemctl --user status kylin-agent-runtime-gateway.service

# Enable or disable automatic startup
systemctl --user enable kylin-agent-runtime-gateway.service
systemctl --user disable kylin-agent-runtime-gateway.service

# Follow service logs
journalctl --user -u kylin-agent-runtime-gateway.service -f
```

For a system-level service, omit `--user` and run the corresponding `systemctl` commands with `sudo`.

To see all commands and options:

```bash
kylin-agent-runtime --help
```

## Development Install

Install the development dependencies when working on the codebase or running tests:

```bash
git clone https://gitee.com/openkylin/agent-runtime.git
cd agent-runtime

curl -LsSf https://astral.sh/uv/install.sh | sh
uv venv .venv --python 3.11
source .venv/bin/activate
uv pip install -e ".[all,dev]"
```

## Update

Pull the latest code and reinstall from the project directory:

```bash
git pull
source .venv/bin/activate
uv pip install -e ".[all]"
```

If the dependency lock file changed, sync the locked dependencies:

```bash
uv sync --all-extras
```

## Tests

Run the default test suite:

```bash
scripts/run_tests.sh
```

Or run pytest directly inside an activated virtual environment:

```bash
python -m pytest tests/ -q
```

## License

MIT — see [LICENSE](LICENSE).
