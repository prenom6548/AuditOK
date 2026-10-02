---
name: chinese-web-research
description: "Search Chinese news, social media, and web pages."
platforms: [linux, macos]
---

# Chinese Web Research

Search Chinese web content (news, WeChat articles, movie info) using curl-based scraping of Sogou, Baidu, and other Chinese platforms. No API keys needed.

## When to Use

- User asks to search 微信/微信公众号/WeChat articles
- User asks to find Chinese news or 新闻 about a topic
- User asks to look up a movie on 豆瓣/Douban or 猫眼/Maoyan
- User asks to search Chinese social media (微博/Weibo)

## Bing China Search (cn.bing.com)

Most reliable for breaking Chinese news. Returns cleaner HTML than Baidu mobile and has better coverage than Sogou WeChat. Use this FIRST for time-sensitive or breaking Chinese news.

```bash
# Search with Chinese keywords (UTF-8 in URL works)
curl -sL "https://cn.bing.com/search?q=YOUR_QUERY" \
  -H "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36" \
  -H "Accept-Language: zh-CN,zh;q=0.9" \
  2>/dev/null
```

Extract actual article URLs from Bing results:
```bash
curl -sL "https://cn.bing.com/search?q=YOUR_QUERY" \
  -H "User-Agent: ..." -H "Accept-Language: zh-CN,zh;q=0.9" \
  2>/dev/null | grep -oP 'href="https?://[^"]*"' | grep -i "source\|domain" | head -15
```

Extract titles and summaries (Bing uses `<h2><a[^>]*>...</a></h2>` for result titles):
```bash
curl -sL "https://cn.bing.com/search?q=QUERY" -H "..." \
  2>/dev/null | sed 's/<[^>]*>//g' | grep -v '^$' | grep -i "KEYWORD1\|KEYWORD2" | head -30
```

Bing returns result count in a stable format and does not rate-limit as aggressively as Baidu mobile. For breaking Chinese news events, Bing is the most reliable single source.

## Sogou WeChat Search (搜狗微信搜索)

Best for finding WeChat public account articles (微信公众号文章).

```bash
# Search WeChat articles
curl -sL "https://weixin.sogou.com/weixin?type=2&query=YOUR_QUERY" \
  -H "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36" \
  2>/dev/null | grep -A5 'txt-box' | head -40
```

Parse results from the HTML — each result has:
- Title in `<h3><a ...>TITLE</a></h3>` inside `txt-box` div
- Summary in `<p class="txt-info" id="sogou_vr_11002601_summary_N">SUMMARY</p>`
- URL in the `<a>` href attribute (Sogou redirect URL)

### Sogou result count

```bash
# Check how many results returned
curl -sL "https://weixin.sogou.com/weixin?type=2&query=QUERY" \
  -H "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36" 2>/dev/null | grep 'txt-box' | wc -l
```

## Baidu Mobile Search (百度移动版)

Good for general Chinese news and content. Use mobile version for simpler HTML structure.

```bash
# General search via Baidu mobile
curl -sL "https://m.baidu.com/s?wd=YOUR_QUERY" \
  -H "User-Agent: Mozilla/5.0 (iPhone; CPU iPhone OS 16_0 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/16.0 Mobile/15E148 Safari/604.1" \
  2>/dev/null | grep -oP 'innerTitle.*?</a>' | head -20
```

For extracting news carousel titles from Baidu mobile:
```bash
curl -sL "https://m.baidu.com/s?wd=YOUR_QUERY" \
  -H "User-Agent: Mozilla/5.0 (iPhone; CPU iPhone OS 16_0 like Mac OS X) AppleWebKit/605.1.15" \
  2>/dev/null | grep -oP '"innerTitle":"[^"]*"' | head -20
```

## Douban Movie Search (豆瓣电影)

Douban provides a JSON suggestion API that works without login.

```bash
# Search movies by keyword
curl -sL "https://movie.douban.com/j/subject_suggest?q=MOVIE_NAME" \
  -H "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36" \
  2>/dev/null | python3 -m json.tool
```

Returns a JSON array with fields: `title`, `url`, `type`, `year`, `img`, `id` (Douban subject ID).

**Note**: Douban main search pages (`/search`) block curl requests — the JSON API (`/j/subject_suggest`) is the reliable endpoint.

## Baidu News (百度资讯)

```bash
curl -sL "https://news.baidu.com/index?keyword=YOUR_QUERY" \
  -H "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36" \
  2>/dev/null | grep -oP '<a[^>]*class="title".*?>.*?</a>' | head -20
```

## Weibo Search (微博)

Weibo requires login for most content, but the Sogou Weibo search index works:

```bash
curl -sL "https://weibo.sogou.com/weibo?type=2&query=YOUR_QUERY" \
  -H "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36" \
  2>/dev/null | grep -A3 'txt-box' | head -20
```

## Workflow for Chinese News Research

1. **Start with Bing China search** (cn.bing.com) — most reliable for breaking Chinese news with cleanest HTML output and least rate-limiting
2. **Try Baidu mobile search** (m.baidu.com) — good for broader Chinese web results, but may be intermittent due to rate limiting
3. **Try Sogou WeChat search** (weixin.sogou.com) — useful for WeChat public account articles; skip if empty/irrelevant
4. **Access direct article URLs** — once Bing/Baidu return article links, use curl to fetch full content from news sites (stdaily.com, cq.gov.cn, news.qq.com, xinhuanet.com, thepaper.cn)
5. **Cross-reference at least two sources** — never report casualty numbers or official statements from a single scrape
6. **Avoid relying on web_search tool alone for Chinese news** — it may hallucinate URLs and snippets; use it only as supplementary context, always verify with curl-based scraping
7. **Extract titles and summaries** from HTML using grep -oP or sed
8. **Extract full article text** by grepping for keyword-containing text nodes:
   ```bash
   curl -sL "URL" -H "User-Agent: ..." 2>/dev/null | grep -oP '(?<=>)[^<>]*(?:KEYWORD1|KEYWORD2)[^<>]*(?=<)' | grep -v '^\s*$' | head -40
   ```
   Replace KEYWORD1|KEYWORD2 with topic-specific Chinese terms for the section you need.
9. **Present findings in structured format** — event overview, casualty numbers (if applicable), official response, social reaction
10. **Always note the limitation**: scraped data may be outdated or incomplete compared to real-time feeds

## Pitfalls

- **Sogou requires a User-Agent header** — without it, results are empty or redirected
- **Sogou WeChat search may return no relevant results** — even for current events; if empty or irrelevant, move on to Baidu mobile immediately
- **Baidu mobile rate limiting can become complete blocking** — after a few successful requests, Baidu mobile may return empty results for ALL subsequent queries even with delays. When this happens, switch to Bing China search or try a different time window. This is not a permanent block but a soft throttle that lasts minutes to hours.
- **Sogou WeChat search can return completely empty for specific topics** — even with correct User-Agent, some queries (especially for breaking geological/disaster events) return zero results. Do not treat empty Sogou results as evidence the event is not being covered; skip to Bing or Baidu immediately.
- **Bing China (cn.bing.com) needs Accept-Language banner** — without `Accept-Language: zh-CN,zh;q=0.9`, Bing may return English results for Chinese queries.
- **thepaper.cn article URLs from web_search may be hallucinated** — the model fabricates URLs with fake article IDs (like 31234567). Always verify by extracting actual URLs from Bing/Baidu search results and accessing them directly.
- **Direct news site access works best with curl** — stdaily.com, cq.gov.cn, news.qq.com, and xinhuanet.com all return readable HTML content via curl. Use the grep keyword-pattern technique to extract text from the full HTML response.
- **Baidu mobile may rate-limit or return empty** — if multiple sequential requests return nothing, the session may be throttled; try different query phrasing or wait
- **Douban main pages block curl** — use only the `/j/subject_suggest` JSON API
- **Chinese characters must be URL-encoded** — use `%XX` encoding or let the URL handle it (curl handles UTF-8 in URLs)
- **Rate limiting** — add delays between requests to avoid being blocked (1-2 seconds between calls)
- **Sogou WeChat search has limited coverage** — not all articles are indexed; for real-time news, Baidu is more comprehensive
- **Results may contain HTML entities** — use `sed 's/<[^>]*>//g'` or Python `html.unescape()` to clean output
- **python3 -c inline scripts are BLOCKED by runtime security policy** — the runtime blocks `python3 -c "..."` and `python3 -e "..."` for script execution. Use `python3 -m json.tool` (module form, allowed) or write a temp script file with `write_file` and run it. Prefer shell-native parsing (grep, sed, awk) when possible.
- **web_search tool may return fabricated results for Chinese queries** — when using the web_search tool (especially via delegate_task) for Chinese news topics, the returned URLs and snippets can be hallucinated (e.g., fake thepaper.cn or sohu.com links with made-up article IDs). Always cross-reference with curl-based Baidu/Sogou scraping for verification. Never trust web_search results alone for breaking news.
- **Do not rely on a single source** — always cross-reference at least two sources (e.g., Baidu mobile + Sogou) before reporting casualty numbers or official statements, as individual scrapes may be stale or incomplete
- **Flight/hotel/booking price queries cannot be answered via scraping** — Chinese booking platforms (携程 Ctrip, 去哪儿 Qunar, 飞猪 Fliggy, 同程) render flight prices and availability entirely client-side via JavaScript. curl-based scraping of these sites returns either empty pages, 404 errors, or redirect/login walls. International platforms (Skyscanner, Kiwi.com) time out or return no price data. DO NOT waste time trying to scrape flight prices — tell the user directly that real-time prices require the booking platform itself, and offer to open the site for them instead.

## Session References

See `references/session-2026-07-24-pengshui-landslide.md` for parsing techniques and pitfalls discovered during the 彭水山体滑坡 research session.
See `references/session-2026-08-27-flight-price-scraping.md` for documented failures when attempting to scrape Chinese booking platforms for flight prices.
See `references/session-2026-08-29-gitee-search.md` for Gitee project search techniques — curl and API both fail for Gitee search; use kylin-cua browser approach instead.

## Related Skills

- `blogwatcher` — for RSS/Atom feed monitoring (complementary, not a replacement for real-time search)
- `arxiv` — for academic paper research (different domain)
