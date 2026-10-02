# Service Management

KylinBot ships with first-class service integration for systemd (Linux), launchctl (macOS), and Task Scheduler / Windows Service (Windows). All three are driven by one CLI surface:

```bash
kylinbot service install     # register the service
kylinbot service start       # start it
kylinbot service stop        # stop it
kylinbot service restart     # stop + start
kylinbot service status      # running / stopped, last exit code
kylinbot service uninstall   # remove it
```

The platform-specific backends are implemented in `crates/kylinbot-runtime/src/service/`. You don't have to think about them — but knowing what they produce helps when debugging.

## Linux — systemd

`kylinbot service install` writes a user-scoped unit at `~/.config/systemd/user/kylinbot.service`.

The unit:

- `Type=simple` with the agent process staying in the foreground
- `User=` set to the invoking user
- `SupplementaryGroups=gpio spi i2c` (enabled if hardware feature is compiled in)
- `Restart=on-failure` with a 10-second backoff
- `ExecStart=/home/$USER/.cargo/bin/kylinbot daemon`

### Manual control

```bash
systemctl --user start kylinbot
systemctl --user stop kylinbot
systemctl --user status kylinbot
systemctl --user enable kylinbot     # start on login
```

### Logs

```bash
journalctl --user -u kylinbot -f        # follow
journalctl --user -u kylinbot --since "1h ago"
```

### System-scope (root) service

If you need KylinBot to start before user login (headless SBCs, VPSes), run the install command as root:

```bash
sudo kylinbot service install
sudo systemctl enable --now kylinbot
```

When invoked with sudo/root, `kylinbot service install` creates a system-scope unit at `/etc/systemd/system/kylinbot.service` and provisions a dedicated `kylinbot` service user.

## Linux — OpenRC

Detected automatically when `/run/openrc` exists (Alpine, some Gentoo configs).

```bash
kylinbot service install   # writes /etc/init.d/kylinbot
rc-service kylinbot start
rc-update add kylinbot default    # start on boot
```

## macOS — LaunchAgent

`kylinbot service install` writes `~/Library/LaunchAgents/com.kylinbot.daemon.plist` and loads it.

```bash
launchctl list | grep kylinbot
launchctl unload ~/Library/LaunchAgents/com.kylinbot.daemon.plist
launchctl load ~/Library/LaunchAgents/com.kylinbot.daemon.plist
```

Logs go to `~/Library/Logs/KylinBot/kylinbot.log` (stdout) and `kylinbot.err` (stderr).

### Homebrew-managed

If installed via Homebrew, `brew services` is the preferred interface:

```bash
brew services start kylinbot
brew services restart kylinbot
brew services info kylinbot
```

Don't mix `kylinbot service` CLI commands with `brew services` — pick one. Both end up writing a plist; having both around confuses `launchctl`.

## Windows — Task Scheduler

`kylinbot service install` creates a scheduled task in the current user's session:

- Trigger: at logon
- Condition: battery, idle, and power-save conditions are **all disabled** (otherwise the task would stop unexpectedly)
- Action: run `kylinbot daemon` hidden

Verify in Task Scheduler GUI (`taskschd.msc`) under Task Scheduler Library → KylinBot.

Logs go to `%LOCALAPPDATA%\KylinBot\logs\`:

```cmd
type %LOCALAPPDATA%\KylinBot\logs\kylinbot.log
```

### Windows Service (system-scope)

For servers or multi-user Windows installs, run `kylinbot service install` from an Administrator prompt:

```cmd
:: Administrator cmd.exe
kylinbot service install
```

Running elevated causes the installer to register a real Windows Service under `LocalSystem` instead of a user-scoped scheduled task. Control via `services.msc` or:

```cmd
sc query KylinBot
sc start KylinBot
sc stop KylinBot
```

## Config path resolution

The service reads config from whichever workspace it was installed against. Order:

1. `$KYLINBOT_CONFIG_DIR/config.toml` if set
2. `$KYLINBOT_WORKSPACE/.kylinbot/config.toml` if set
3. `$HOMEBREW_PREFIX/var/kylinbot/.kylinbot/config.toml` if installed via Homebrew
4. `~/.kylinbot/config.toml` (Linux/macOS) or `%USERPROFILE%\.kylinbot\config.toml` (Windows)

If your service seems to ignore config changes, check which path the daemon is reading:

```bash
kylinbot config list
```

The first few lines of its output show the config file path it resolved against.

## Auto-update

The service does **not** auto-update. That's deliberate — you pick when to take new code. Subscribe to the GitHub release feed or the Discord `#releases` channel (see [Contributing → Communication](../contributing/communication.md)).

## See also

- [Linux setup](./linux.md), [macOS setup](./macos.md), [Windows setup](./windows.md)
- [Operations → Logs & observability](../ops/observability.md)
- [Operations → Troubleshooting](../ops/troubleshooting.md)
