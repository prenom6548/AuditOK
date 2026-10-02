use crate::cli::Cli;

pub fn host_display(cli: &Cli, host_ascii: &str) -> String {
    if !cli.effective_enable_iri() {
        return host_ascii.to_string();
    }
    let (uni, _errors) = idna::domain_to_unicode(host_ascii);
    if uni.eq_ignore_ascii_case(host_ascii) {
        host_ascii.to_string()
    } else {
        // GNU 日志常见形态：unicode (ascii)
        format!("{uni} ({host_ascii})")
    }
}
