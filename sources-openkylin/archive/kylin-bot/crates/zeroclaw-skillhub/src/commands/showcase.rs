use anyhow::Result;
use colored::Colorize;

use crate::client::ApiClient;

pub fn normalize_scene(scene: &str) -> &str {
    match scene {
        "hot" | "trending" | "ranking" => "hot",
        "featured" | "recommend" | "recommended" => "featured",
        "newest" | "recent" | "new" | "latest" => "newest",
        other => other,
    }
}

pub async fn run(api: &ApiClient, category: &str, json_output: bool) -> Result<()> {
    let scene = normalize_scene(category);
    let resp = api.showcase(scene).await?;

    if json_output {
        println!("{}", serde_json::to_string_pretty(&resp)?);
        return Ok(());
    }

    if resp.skills.is_empty() {
        println!(
            "No skills in showcase '{}'. The category may not exist or the ClawHub API is unavailable.",
            category
        );
        return Ok(());
    }

    let title = match scene {
        "hot" => "🔥 下载热榜",
        "featured" => "⭐ 为你推荐",
        "newest" => "🆕 最近上新",
        _ => scene,
    };
    println!("{}\n", title.bold());

    for (i, skill) in resp.skills.iter().enumerate() {
        let rank = format!("{:>2}.", i + 1).dimmed();
        println!(
            "{} {} {}",
            rank,
            skill.slug.bold(),
            format!("({})", skill.name).dimmed()
        );
        if !skill.description_zh.is_empty() {
            println!(
                "    {}",
                skill.description_zh.chars().take(100).collect::<String>()
            );
        } else if !skill.description.is_empty() {
            println!(
                "    {}",
                skill.description.chars().take(100).collect::<String>()
            );
        }
        let mut stats = Vec::new();
        if skill.downloads > 0 {
            stats.push(format!("⬇ {}", format_downloads(skill.downloads)));
        }
        if skill.stars > 0 {
            stats.push(format!("⭐ {}", skill.stars));
        }
        if !stats.is_empty() {
            println!("    {}", stats.join("  ").dimmed());
        }
    }

    println!("\n{}", format!("共 {} 个技能", resp.skills.len()).dimmed());
    Ok(())
}

fn format_downloads(n: u64) -> String {
    if n >= 1_000_000 {
        format!("{:.1}M", n as f64 / 1_000_000.0)
    } else if n >= 1_000 {
        format!("{:.1}K", n as f64 / 1_000.0)
    } else {
        n.to_string()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_normalize_scene_hot_aliases() {
        assert_eq!(normalize_scene("hot"), "hot");
        assert_eq!(normalize_scene("trending"), "hot");
        assert_eq!(normalize_scene("ranking"), "hot");
    }

    #[test]
    fn test_normalize_scene_featured_aliases() {
        assert_eq!(normalize_scene("featured"), "featured");
        assert_eq!(normalize_scene("recommend"), "featured");
        assert_eq!(normalize_scene("recommended"), "featured");
    }

    #[test]
    fn test_normalize_scene_newest_aliases() {
        assert_eq!(normalize_scene("newest"), "newest");
        assert_eq!(normalize_scene("recent"), "newest");
        assert_eq!(normalize_scene("new"), "newest");
        assert_eq!(normalize_scene("latest"), "newest");
    }

    #[test]
    fn test_normalize_scene_passthrough() {
        assert_eq!(normalize_scene("custom-category"), "custom-category");
        assert_eq!(normalize_scene("other"), "other");
    }

    #[test]
    fn test_format_downloads_units() {
        assert_eq!(format_downloads(0), "0");
        assert_eq!(format_downloads(999), "999");
        assert_eq!(format_downloads(1_000), "1.0K");
        assert_eq!(format_downloads(1_500), "1.5K");
        assert_eq!(format_downloads(999_999), "1000.0K");
        assert_eq!(format_downloads(1_000_000), "1.0M");
        assert_eq!(format_downloads(2_500_000), "2.5M");
    }
}
