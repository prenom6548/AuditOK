use std::fs::{File, OpenOptions};
use std::io::{self, Write};

use crate::cli::Cli;
use crate::exit::ExitCode;
use crate::log;

pub enum OutputTarget {
    Stdout,
    File { path: String, append: bool },
}

pub fn open_output(
    target: OutputTarget,
    cli: &Cli,
    restarted: bool,
) -> Result<Box<dyn Write>, ExitCode> {
    match target {
        OutputTarget::Stdout => Ok(Box::new(io::stdout().lock())),
        OutputTarget::File { path, append } => {
            if restarted && !cli.quiet {
                log::log_status_line(
                    cli,
                    crate::i18n::gettext(
                        "wget: server ignored Range; restarting download at byte 0",
                    ),
                );
            }
            let f: File = if append {
                OpenOptions::new()
                    .create(true)
                    .append(true)
                    .open(&path)
                    .map_err(|_| ExitCode::IoFail)?
            } else {
                File::create(&path).map_err(|_| ExitCode::IoFail)?
            };
            Ok(Box::new(f))
        }
    }
}
