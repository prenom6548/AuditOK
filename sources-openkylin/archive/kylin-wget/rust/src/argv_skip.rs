//! 扫描 `argv` 时跳过「吃下一个 token」的选项，避免把 URL/文件名误判成子开关。

pub(crate) fn skip_option_slots(args: &[String], i: usize) -> usize {
    let a = &args[i];
    if a.starts_with("--") && a.contains('=') {
        return 1;
    }
    match a.as_str() {
        "-O" | "-o" | "-a" | "-t" | "-T" | "-i" | "-B" | "-U" | "-n" | "-P" => {
            if i + 1 < args.len() { 2 } else { 1 }
        }
        "--output-document" | "--output-file" | "--append-output" | "--tries" | "--timeout"
        | "--connect-timeout" | "--read-timeout" | "--max-redirect" | "--limit-rate"
        | "--input-file" | "--base" | "--user-agent" | "--referer" | "--directory-prefix"
        | "--method" | "--body-data" | "--body-file" | "--post-data" | "--post-file"
        | "--proxy-user" | "--proxy-password" | "--http-user" | "--http-password" | "--user"
        | "--password" | "--start-pos" | "--cut-dirs" => {
            if i + 1 < args.len() {
                2
            } else {
                1
            }
        }
        "--header" | "--progress" => {
            if i + 1 < args.len() {
                2
            } else {
                1
            }
        }
        _ => 1,
    }
}
