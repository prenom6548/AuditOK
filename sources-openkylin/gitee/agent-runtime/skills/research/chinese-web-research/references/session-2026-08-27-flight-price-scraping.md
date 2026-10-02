# Flight Price Scraping — Documented Failures (2026-08-27)

**Task**: User asked for cheapest flight prices from Changsha (长沙) to Hainan (海南) during Mid-Autumn Festival 2025 (Oct 6).

## What was tried and why it failed

| Platform | Approach | Result |
|---|---|---|
| 携程 Ctrip (ctrip.com) | curl desktop flight list page, grep for price | Empty — prices rendered client-side via JS |
| 携程 Ctrip mobile API | `m.ctrip.com/restapi/soa2/12345/getList` | 404 — "Not-Found-Route" |
| 飞猪 Fliggy | curl `m.fliggy.com/domesticflight/list` | Redirected to Taobao 404 page |
| Skyscanner | curl flight search URL | Connection timeout (30s) |
| Kiwi.com | curl flight search URL, grep `"price"` | Empty response, no price data |
| Bing CN search | Search for "长沙 海南 机票 中秋节 最便宜" | No flight price data in snippets — only travel guides, city info, and general articles |
| Baidu mobile search | Search for "长沙去海南机票中秋节最便宜" | No price snippets — only JS config, no structured results |

## Key takeaway

Chinese booking platforms (携程、去哪儿、飞猪、同程) render all pricing data client-side. curl-based scraping returns:
- Empty pages with only JS bundles
- 404 errors on API endpoints (endpoints change frequently, undocumented)
- Login/redirect walls on mobile pages

**Correct response**: Tell the user that real-time flight prices cannot be retrieved via scraping. Suggest they check the booking platform directly. Offer to open the site for them using kylin-cua if needed.
