//! GNU wget `progress.c`：`--progress=bar|dot[:style]`；非 TTY 默认回退 dot。

use crate::cli::Cli;
use crate::log;
use std::io::IsTerminal;
use std::path::Path;

const REFRESH_INTERVAL: f64 = 0.2;
const DEFAULT_WIDTH: usize = 80;
const MIN_WIDTH: usize = 40;

/// GNU dot 样式参数（`--progress=dot:default|binary|mega|giga`）。
#[derive(Clone, Copy, Debug)]
pub struct DotConfig {
    pub dot_bytes: u64,
    pub dot_spacing: u32,
    pub dots_in_line: u32,
}

impl DotConfig {
    pub const fn default_style() -> Self {
        Self {
            dot_bytes: 1024,
            dot_spacing: 10,
            dots_in_line: 50,
        }
    }

    const fn binary() -> Self {
        Self {
            dot_bytes: 8192,
            dot_spacing: 16,
            dots_in_line: 48,
        }
    }

    const fn mega() -> Self {
        Self {
            dot_bytes: 65536,
            dot_spacing: 8,
            dots_in_line: 48,
        }
    }

    const fn giga() -> Self {
        Self {
            dot_bytes: 1 << 20,
            dot_spacing: 8,
            dots_in_line: 32,
        }
    }

    fn from_style_name(name: &str) -> Option<Self> {
        match name.to_ascii_lowercase().as_str() {
            "default" => Some(Self::default_style()),
            "binary" => Some(Self::binary()),
            "mega" => Some(Self::mega()),
            "giga" => Some(Self::giga()),
            _ => None,
        }
    }
}

enum ResolvedProgress {
    Bar,
    Dot(DotConfig),
}

/// 校验 `--progress` 类型名（GNU `valid_progress_implementation_p`）。
pub fn validate_progress_type(spec: &str) -> Result<(), String> {
    let kind = spec.split(':').next().unwrap_or("").trim();
    if kind.is_empty() {
        return Err(crate::i18n::gettext("invalid progress indicator ''").to_string());
    }
    if kind.eq_ignore_ascii_case("bar") || kind.eq_ignore_ascii_case("dot") {
        Ok(())
    } else {
        Err(crate::i18n::tr_named(
            "invalid progress indicator '{kind}'",
            &[("kind", kind)],
        ))
    }
}

fn resolve_progress(cli: &Cli) -> ResolvedProgress {
    let Some(spec) = cli.progress.as_deref() else {
        return if stderr_is_tty() {
            ResolvedProgress::Bar
        } else {
            ResolvedProgress::Dot(DotConfig::default_style())
        };
    };

    let mut parts = spec.split(':');
    let kind = parts.next().unwrap_or("").trim();
    match kind.to_ascii_lowercase().as_str() {
        "dot" => {
            let mut cfg = DotConfig::default_style();
            for param in parts {
                let p = param.trim();
                if p.is_empty() {
                    continue;
                }
                if let Some(style) = DotConfig::from_style_name(p) {
                    cfg = style;
                } else {
                    eprintln!(
                        "{}",
                        crate::i18n::tr_named(
                            "wget: invalid dot style specification {style}; leaving unchanged.",
                            &[("style", p)],
                        )
                    );
                }
            }
            ResolvedProgress::Dot(cfg)
        }
        "bar" => {
            let mut force = false;
            for param in parts {
                let p = param.trim();
                if p.eq_ignore_ascii_case("force") {
                    force = true;
                }
            }
            if !force && !stderr_is_tty() {
                ResolvedProgress::Dot(DotConfig::default_style())
            } else {
                ResolvedProgress::Bar
            }
        }
        _ => ResolvedProgress::Bar,
    }
}

pub struct DownloadProgress {
    inner: ProgressInner,
}

enum ProgressInner {
    Bar(BarProgress),
    Dot(DotProgress),
}

impl DownloadProgress {
    pub fn new(cli: &Cli, total: Option<u64>, _label: Option<&str>) -> Self {
        let inner = match resolve_progress(cli) {
            ResolvedProgress::Bar => ProgressInner::Bar(BarProgress::new(total)),
            ResolvedProgress::Dot(cfg) => ProgressInner::Dot(DotProgress::new(total, cfg)),
        };
        Self { inner }
    }

    pub fn begin_transfer(&mut self, cli: &Cli) {
        match &mut self.inner {
            ProgressInner::Bar(b) => b.begin(cli),
            ProgressInner::Dot(d) => d.begin_transfer(cli),
        }
    }

    pub fn update(&mut self, cli: &Cli, howmuch: u64, dltime: f64) {
        match &mut self.inner {
            ProgressInner::Bar(b) => b.update(cli, howmuch, dltime),
            ProgressInner::Dot(d) => d.update(cli, howmuch, dltime),
        }
    }

    pub fn finish(&mut self, cli: &Cli, dltime: f64) {
        match &mut self.inner {
            ProgressInner::Bar(b) => b.finish(cli, dltime),
            ProgressInner::Dot(d) => d.finish(cli, dltime),
        }
    }
}

fn stderr_is_tty() -> bool {
    std::io::stderr().is_terminal()
}

fn screen_width() -> usize {
    if let Ok(cols) = std::env::var("COLUMNS") {
        if let Ok(n) = cols.parse::<usize>() {
            return n.clamp(MIN_WIDTH, 200).saturating_sub(1);
        }
    }
    if let Ok(out) = std::process::Command::new("stty")
        .args(["size"])
        .output()
    {
        if let Ok(text) = String::from_utf8(out.stdout) {
            let mut parts = text.split_whitespace();
            if let Some(cols) = parts.nth(1).and_then(|s| s.parse::<usize>().ok()) {
                return cols.clamp(MIN_WIDTH, 200).saturating_sub(1);
            }
        }
    }
    DEFAULT_WIDTH.saturating_sub(1)
}

pub fn label_from_output_path(out: &str) -> Option<String> {
    if out == "-" {
        return None;
    }
    Some(
        Path::new(out)
            .file_name()
            .and_then(|s| s.to_str())
            .unwrap_or(out)
            .to_string(),
    )
}

struct BarProgress {
    total_length: Option<u64>,
    count: u64,
    width: usize,
    last_screen_update: f64,
    started: bool,
}

impl BarProgress {
    fn new(total: Option<u64>) -> Self {
        Self {
            total_length: total,
            count: 0,
            width: screen_width(),
            last_screen_update: -1.0,
            started: false,
        }
    }

    fn begin(&mut self, cli: &Cli) {
        if !cli.effective_show_progress() || cli.quiet {
            return;
        }
        // 文件名/保存路径已在 log_length_* 中换行完整输出；此处仅刷新进度条行。
        self.render(cli, 0.0, false, true);
        self.started = true;
    }

    fn update(&mut self, cli: &Cli, howmuch: u64, dltime: f64) {
        if howmuch == 0 || !cli.effective_show_progress() || cli.quiet {
            return;
        }
        self.count = self.count.saturating_add(howmuch);
        if !self.started {
            self.begin(cli);
        }
        if dltime - self.last_screen_update < REFRESH_INTERVAL {
            return;
        }
        self.render(cli, dltime, false, false);
        self.last_screen_update = dltime;
    }

    fn finish(&mut self, cli: &Cli, dltime: f64) {
        if !cli.effective_show_progress() || cli.quiet {
            return;
        }
        if !self.started {
            self.begin(cli);
        }
        self.render(cli, dltime, true, false);
        log::progress_write_partial(cli, "\n\n");
    }

    fn render(&self, cli: &Cli, dltime: f64, done: bool, _force: bool) {
        let line = self.build_line(dltime, done);
        log::progress_write_partial(cli, &format!("\r{line}"));
    }

    fn build_line(&self, dltime: f64, done: bool) -> String {
        let total = self.total_length.filter(|t| *t > 0);
        let pct = total
            .map(|t| ((self.count as f64 * 100.0 / t as f64).floor() as u64).min(100))
            .unwrap_or(0);

        let tail = build_suffix(self.count, total, dltime, done);
        let tail_len = tail.chars().count();
        let pct_str = if total.is_some() {
            format!("{pct:>3}%")
        } else {
            "   %".to_string()
        };
        let pct_len = pct_str.chars().count();

        const MIN_BAR: usize = 15;
        let bar_w = self
            .width
            .saturating_sub(tail_len + pct_len)
            .clamp(MIN_BAR, self.width.saturating_sub(tail_len + pct_len));

        let bar = build_bar(self.count, total, bar_w);
        let mut line = format!("{pct_str}{bar}{tail}");
        while line.chars().count() < self.width {
            line.push(' ');
        }
        line
    }
}

fn build_bar(received: u64, total: Option<u64>, bar_w: usize) -> String {
    let mut bar = String::with_capacity(bar_w + 2);
    bar.push('[');
    let filled = total
        .map(|t| {
            let pos = (received as f64 / t as f64 * bar_w as f64).floor() as usize;
            pos.clamp(0, bar_w)
        })
        .unwrap_or(0);

    // 对齐 GNU：`[=====>     ]`，`>` 在已下载区域末端。
    if filled == 0 {
        bar.push('>');
    } else {
        for _ in 0..filled.saturating_sub(1) {
            bar.push('=');
        }
        bar.push('>');
    }
    while bar.len() - 1 < bar_w {
        bar.push(' ');
    }
    bar.push(']');
    bar
}

fn build_suffix(received: u64, total: Option<u64>, dltime: f64, done: bool) -> String {
    let size = format!("  {}", fmt_human_size(received));
    let speed = if dltime > 0.0 {
        format!("  {}", fmt_speed(received as f64 / dltime))
    } else {
        "  --.-KB/s".to_string()
    };
    if done {
        let tail = if dltime >= 10.0 {
            format!("   in {}", eta_to_human_short(dltime.round() as i32, false))
        } else {
            format!("   in {:.0}s", dltime.max(0.0))
        };
        return format!("{size}{speed}{tail}");
    }

    let eta = if let Some(t) = total {
        if received > 0 && dltime > 3.0 {
            let remaining = t.saturating_sub(received);
            let eta_secs = (dltime * remaining as f64 / received as f64).round() as i32;
            if eta_secs < i32::MAX - 1 {
                Some(eta_secs)
            } else {
                None
            }
        } else {
            None
        }
    } else {
        None
    };

    match eta {
        Some(secs) => format!(
            "{size}{speed}  {}{}",
            eta_prefix(),
            eta_to_human_short(secs, false)
        ),
        None if total.is_some() && !done => format!("{size}{speed}              "),
        None => format!("{size}{speed}"),
    }
}

fn eta_prefix() -> &'static str {
    crate::i18n::gettext("eta ")
}

fn fmt_human_size(bytes: u64) -> String {
    const KB: f64 = 1024.0;
    const MB: f64 = KB * 1024.0;
    const GB: f64 = MB * 1024.0;
    let b = bytes as f64;
    if b >= GB {
        format!("{:.2}G", b / GB)
    } else if b >= MB {
        format!("{:.2}M", b / MB)
    } else if b >= KB {
        format!("{:.2}K", b / KB)
    } else {
        format!("{b:.0}B")
    }
}

fn fmt_speed(bytes_per_sec: f64) -> String {
    const KB: f64 = 1024.0;
    const MB: f64 = KB * 1024.0;
    const GB: f64 = MB * 1024.0;
    if bytes_per_sec >= GB {
        format!("{:.1}GB/s", bytes_per_sec / GB)
    } else if bytes_per_sec >= MB {
        format!("{:.1}MB/s", bytes_per_sec / MB)
    } else if bytes_per_sec >= KB {
        format!("{:.1}KB/s", bytes_per_sec / KB)
    } else {
        format!("{:.0}B/s", bytes_per_sec.max(0.0))
    }
}

struct DotProgress {
    dot_bytes: u64,
    dot_spacing: u32,
    dots_in_line: u32,
    initial_length: u64,
    total_length: Option<u64>,
    accumulated: u64,
    rows: u64,
    dots: u32,
    last_timer_value: f64,
    row_header_pending: bool,
}

impl DotProgress {
    fn new(total: Option<u64>, cfg: DotConfig) -> Self {
        Self {
            dot_bytes: cfg.dot_bytes,
            dot_spacing: cfg.dot_spacing,
            dots_in_line: cfg.dots_in_line,
            initial_length: 0,
            total_length: total,
            accumulated: 0,
            rows: 0,
            dots: 0,
            last_timer_value: 0.0,
            row_header_pending: true,
        }
    }

    fn begin_transfer(&mut self, cli: &Cli) {
        if !cli.effective_show_progress() || cli.quiet {
            return;
        }
        if self.row_header_pending && self.dots == 0 {
            self.print_row_header(cli);
            self.row_header_pending = false;
        }
    }

    fn row_bytes(&self) -> u64 {
        self.dot_bytes * self.dots_in_line as u64
    }

    fn update(&mut self, cli: &Cli, howmuch: u64, dltime: f64) {
        if howmuch == 0 || !cli.effective_show_progress() || cli.quiet {
            return;
        }
        self.accumulated = self.accumulated.saturating_add(howmuch);
        while self.accumulated >= self.dot_bytes {
            self.accumulated -= self.dot_bytes;
            if self.dots == 0 && self.row_header_pending {
                self.print_row_header(cli);
                self.row_header_pending = false;
            }
            if self.dots % self.dot_spacing == 0 {
                log::progress_write_partial(cli, " ");
            }
            log::progress_write_partial(cli, ".");
            self.dots += 1;
            if self.dots >= self.dots_in_line {
                self.rows += 1;
                self.dots = 0;
                self.row_header_pending = true;
                self.print_row_stats(cli, dltime, false);
            }
        }
    }

    fn finish(&mut self, cli: &Cli, dltime: f64) {
        if !cli.effective_show_progress() || cli.quiet {
            return;
        }
        if self.dots == 0 && self.row_header_pending {
            self.print_row_header(cli);
            self.row_header_pending = false;
        }
        for i in self.dots..self.dots_in_line {
            if i % self.dot_spacing == 0 {
                log::progress_write_partial(cli, " ");
            }
            log::progress_write_partial(cli, " ");
        }
        self.print_row_stats(cli, dltime, true);
        log::progress_write_partial(cli, "\n\n");
    }

    fn print_row_header(&self, cli: &Cli) {
        let k = self.rows.saturating_mul(self.row_bytes()) / 1024;
        log::progress_write_partial(cli, &format!("\n{k:>6}K"));
    }

    fn print_row_stats(&mut self, cli: &Cli, dltime: f64, last: bool) {
        let row_bytes = self.row_bytes();
        let mut bytes_displayed = self
            .rows
            .saturating_mul(row_bytes)
            .saturating_add(u64::from(self.dots) * self.dot_bytes);
        if last {
            bytes_displayed = bytes_displayed.saturating_add(self.accumulated);
        }

        let mut line = String::new();
        if let Some(total) = self.total_length {
            if total > 0 {
                let pct = (100.0 * bytes_displayed as f64 / total as f64).floor() as u64;
                line.push_str(&format!("{:>3}%", pct.min(100)));
            }
        }

        let mut bytes_this_row = if last {
            u64::from(self.dots) * self.dot_bytes + self.accumulated
        } else {
            row_bytes
        };
        if self.rows == self.initial_length / row_bytes {
            bytes_this_row = bytes_this_row.saturating_sub(self.initial_length % row_bytes);
        }
        let interval = (dltime - self.last_timer_value).max(0.0);
        let (rate, unit) = calc_rate(bytes_this_row, interval);
        let prec = if rate >= 99.95 {
            0
        } else if rate >= 9.995 {
            1
        } else {
            2
        };
        const UNITS: [char; 4] = [' ', 'K', 'M', 'G'];
        line.push_str(&format!(" {rate:4.prec$}{}", UNITS[unit as usize]));
        self.last_timer_value = dltime;

        if !last {
            if let Some(total) = self.total_length {
                if total > 0 {
                    let bytes_remaining = total.saturating_sub(bytes_displayed);
                    let bytes_sofar = bytes_displayed.saturating_sub(self.initial_length);
                    if bytes_sofar > 0 {
                        let eta = dltime * bytes_remaining as f64 / bytes_sofar as f64;
                        if eta < (i32::MAX - 1) as f64 {
                            line.push(' ');
                            line.push_str(&eta_to_human_short(eta.round() as i32, true));
                        }
                    }
                }
            }
        } else if dltime >= 10.0 {
            line.push('=');
            line.push_str(&eta_to_human_short(dltime.round() as i32, true));
        } else {
            line.push('=');
            line.push_str(&format!("{dltime:.0}s"));
        }

        log::progress_write_partial(cli, &line);
    }
}

fn calc_rate(bytes: u64, secs: f64) -> (f64, u8) {
    if secs <= 0.0 {
        return (0.0, 0);
    }
    let mut rate = bytes as f64 / secs;
    let mut unit = 0u8;
    while rate >= 1024.0 && unit < 3 {
        rate /= 1024.0;
        unit += 1;
    }
    (rate, unit)
}

fn eta_to_human_short(secs: i32, condensed: bool) -> String {
    let space = if condensed { "" } else { " " };
    if secs < 100 {
        format!("{secs}s")
    } else if secs < 100 * 60 {
        format!("{}m{}{}s", secs / 60, space, secs % 60)
    } else if secs < 48 * 3600 {
        format!("{}h{}{}m", secs / 3600, space, (secs / 60) % 60)
    } else if secs < 100 * 86400 {
        format!("{}d{}{}h", secs / 86400, space, (secs / 3600) % 24)
    } else {
        format!("{}d", secs / 86400)
    }
}
