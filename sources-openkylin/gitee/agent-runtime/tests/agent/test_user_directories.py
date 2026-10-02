from pathlib import Path

from agent.user_directories import get_user_directories


def test_linux_uses_localized_xdg_user_directories(tmp_path):
    config = tmp_path / ".config"
    config.mkdir()
    (config / "user-dirs.dirs").write_text(
        'XDG_DESKTOP_DIR="$HOME/桌面"\n'
        'XDG_DOCUMENTS_DIR="$HOME/文档"\n'
        'XDG_DOWNLOAD_DIR="$HOME/下载"\n',
        encoding="utf-8",
    )

    result = get_user_directories(home=tmp_path, env={}, platform="linux")

    assert result == {
        "desktop": tmp_path / "桌面",
        "documents": tmp_path / "文档",
        "downloads": tmp_path / "下载",
    }


def test_linux_honors_xdg_config_home(tmp_path):
    config = tmp_path / "custom-config"
    config.mkdir()
    (config / "user-dirs.dirs").write_text(
        'XDG_DESKTOP_DIR="/data/my desktop"\n', encoding="utf-8"
    )

    result = get_user_directories(
        home=tmp_path,
        env={"XDG_CONFIG_HOME": str(config)},
        platform="linux",
    )

    assert result["desktop"] == Path("/data/my desktop")


def test_missing_config_falls_back_to_conventional_names(tmp_path):
    result = get_user_directories(home=tmp_path, env={}, platform="linux")

    assert result["desktop"] == tmp_path / "Desktop"
    assert result["documents"] == tmp_path / "Documents"
    assert result["downloads"] == tmp_path / "Downloads"


def test_xdg_values_do_not_expand_arbitrary_shell_variables(tmp_path):
    config = tmp_path / ".config"
    config.mkdir()
    (config / "user-dirs.dirs").write_text(
        'XDG_DESKTOP_DIR="$UNTRUSTED/Desktop"\n', encoding="utf-8"
    )

    result = get_user_directories(home=tmp_path, env={}, platform="linux")

    assert result["desktop"] == tmp_path / "Desktop"
