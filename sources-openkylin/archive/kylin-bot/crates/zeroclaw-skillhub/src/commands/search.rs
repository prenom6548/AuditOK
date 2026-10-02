use anyhow::Result;
use colored::Colorize;

use crate::client::ApiClient;

pub async fn run(api: &ApiClient, query: &str, json_output: bool) -> Result<()> {
    let results = api.search(query, None).await?;

    if json_output {
        println!("{}", serde_json::to_string_pretty(&results)?);
        return Ok(());
    }

    if results.is_empty() {
        println!("No results found for '{}'.", query);
        return Ok(());
    }

    println!(
        "Search results for '{}' ({}):\n",
        query.bold(),
        results.len()
    );

    for (i, r) in results.iter().enumerate() {
        println!("  {}. {}", i + 1, r.name.bold());
        println!("     slug:   {}", r.slug.dimmed());
        println!("     desc:   {}", r.description);
        if !r.version.is_empty() {
            println!("     ver:    {}", r.version.dimmed());
        }
        if !r.author.is_empty() {
            println!("     author: {}", r.author);
        }
        if !r.tags.is_empty() {
            println!("     tags:   {}", r.tags.join(", ").dimmed());
        }
        if let Some(d) = r.downloads {
            println!("     dl:     {}", d);
        }
        if let Some(s) = r.stars {
            println!("     stars:  {}", s);
        }
        if let Some(url) = &r.profile_url {
            println!("     url:    {}", url);
        }
        println!();
    }

    Ok(())
}
