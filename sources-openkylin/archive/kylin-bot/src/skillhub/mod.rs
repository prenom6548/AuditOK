use kylinbot::SkillhubCommands;
use kylinbot_config::schema::Config;

pub async fn handle_command(command: SkillhubCommands, config: &Config) -> anyhow::Result<()> {
    let api = zeroclaw_skillhub::ApiClient::new();
    let workspace_dir = &config.workspace_dir;

    match command {
        SkillhubCommands::Search { query, json } => {
            zeroclaw_skillhub::commands::search::run(&api, &query, json).await
        }
        SkillhubCommands::Showcase { category, json } => {
            zeroclaw_skillhub::commands::showcase::run(&api, &category, json).await
        }
        SkillhubCommands::Install {
            source,
            force,
            timeout,
        } => {
            zeroclaw_skillhub::commands::install::run(&api, workspace_dir, &source, force, timeout)
                .await
        }
        SkillhubCommands::List { json } => {
            zeroclaw_skillhub::commands::list::run(workspace_dir, json)
        }
        SkillhubCommands::Upgrade {
            slug,
            check_only,
            timeout,
        } => {
            zeroclaw_skillhub::commands::upgrade::run(
                &api,
                workspace_dir,
                slug.as_deref(),
                check_only,
                timeout,
            )
            .await
        }
        SkillhubCommands::Uninstall { slug } => {
            zeroclaw_skillhub::commands::uninstall::run(workspace_dir, &slug)
        }
    }
}
