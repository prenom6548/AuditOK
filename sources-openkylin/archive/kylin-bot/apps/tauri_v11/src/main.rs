//! KylinBot Desktop — main entry point.
//!
//! Prevents an additional console window on Windows in release.
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

fn main() {
    kylinbot_desktop_v11::run();
}
