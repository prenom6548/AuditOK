#![allow(ambiguous_glob_reexports)]
// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later


pub mod capi;
pub mod core;
pub mod locale;
pub mod paths;

pub use crate::capi::*;
pub use crate::core::*;
pub use crate::locale::*;

// Convenience re-exports matching C++ public surface
pub use crate::core::catalog;
pub use crate::core::kuit;
pub use crate::core::message::Message;
pub use crate::core::plural;
pub use crate::core::transcript;
#[doc(hidden)]
pub use crate::core::global::reset_locale_languages_cache_for_tests;

// Module-style facade so `ki18n_rs::message::Message` works
pub mod message {
    pub use crate::core::message::*;
}

