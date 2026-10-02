/// 与 GNU `src/exits.h` 中 `WGET_EXIT_*` 数值对齐（脚本与 CI 可依赖）。
#[derive(Copy, Clone, Debug, PartialEq, Eq)]
#[repr(i32)]
pub enum ExitCode {
    Success = 0,
    GenericError = 1,
    ParseError = 2,
    IoFail = 3,
    NetworkFail = 4,
    /// `VERIFCERTERR` 等 TLS 证书校验失败（启用校验时）。
    SslAuthFail = 5,
    /// `AUTHFAILED`：已尝试服务端认证（如 HTTP Basic）仍被拒绝。
    ServerAuthFail = 6,
    /// `HEOF` / `HERR` 等协议层错误（预留，按场景逐步映射）。
    #[allow(dead_code)]
    ProtocolError = 7,
    ServerError = 8,
}

/// GNU wget 多 URL 批处理退出码聚合（含 `-nc` 跳过与部分成功）。
///
/// `retrieved == true` 表示该 URL 真正把响应写入了输出文件（非 `-nc` 跳过）。
pub fn aggregate_batch_exit(outcomes: &[(i32, bool)]) -> i32 {
    if outcomes.is_empty() {
        return ExitCode::Success as i32;
    }
    if outcomes.len() == 1 {
        let (code, retrieved) = outcomes[0];
        if code == ExitCode::GenericError as i32 && retrieved {
            return ExitCode::Success as i32;
        }
        return code;
    }

    let mut max_code = ExitCode::Success as i32;
    let mut any_retrieved = false;
    let mut any_explicit_nc_skip = false;

    for &(code, retrieved) in outcomes {
        if retrieved {
            any_retrieved = true;
        }
        if code == ExitCode::GenericError as i32 {
            any_explicit_nc_skip = true;
        }
        if code > max_code {
            max_code = code;
        }
    }

    if max_code >= ExitCode::ServerError as i32 {
        return max_code;
    }
    if max_code >= ExitCode::ServerAuthFail as i32 {
        return max_code;
    }
    if max_code >= ExitCode::NetworkFail as i32 {
        return max_code;
    }
    if any_retrieved {
        return ExitCode::Success as i32;
    }
    if any_explicit_nc_skip {
        return ExitCode::GenericError as i32;
    }
    max_code
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn batch_all_default_nc_skip_is_zero() {
        assert_eq!(aggregate_batch_exit(&[(0, false), (0, false)]), 0);
    }

    #[test]
    fn batch_all_explicit_nc_skip_is_one() {
        assert_eq!(aggregate_batch_exit(&[(1, false), (1, false)]), 1);
    }

    #[test]
    fn batch_skip_then_retrieve_is_zero() {
        assert_eq!(aggregate_batch_exit(&[(1, false), (0, true)]), 0);
    }

    #[test]
    fn batch_skip_and_404_is_eight() {
        assert_eq!(aggregate_batch_exit(&[(1, false), (8, false)]), 8);
    }

    #[test]
    fn single_explicit_nc_skip_is_one() {
        assert_eq!(aggregate_batch_exit(&[(1, false)]), 1);
    }
}
