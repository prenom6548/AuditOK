# 彭水山体滑坡搜索 session notes

Date: 2026-07-24 (second session on this topic)

## Summary of the event

On July 17, 2026, at ~9:08 AM, a massive rock collapse occurred in Pengshui County, Chongqing (彭水苗族土家族自治县汉葭街道, 乌江右岸). ~18,000 cubic meters of rock collapsed initially, growing to 78,000 cubic meters of debris. Largest single boulder was 5,300 cubic meters. 10 people rescued, 8 confirmed dead, 34 initially missing. Level I geological disaster emergency response activated. 872 rescuers, 1,100+ equipment, 13 teams deployed.

## What worked in THIS session

### Bing China search (cn.bing.com) — BEST source in this session

Bing returned clean, structured HTML with actual article URLs. Key technique:

```bash
curl -sL "https://cn.bing.com/search?q=KEYWORDS" \
  -H "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36" \
  -H "Accept-Language: zh-CN,zh;q=0.9" \
  2>/dev/null
```

Extract actual URLs:
```bash
... | grep -oP 'href="https?://[^"]*"' | grep -i "thepaper\|xinhua\|news.qq\|cq.gov\|stdaily\|zhihu" | head -15
```

Extract article titles with context:
```bash
... | sed 's/<[^>]*>//g' | grep -v '^$' | grep -i "彭水\|山体\|崩塌\|救援\|失联\|重庆" | head -30
```

### Direct article URL access

Using URLs from Bing, accessed these sites successfully:
- **stdaily.com** (科技日报) — https://www.stdaily.com/web/gdxw/2026-07/20/content_549786.html
- **cq.gov.cn** (重庆市政府) — https://www.cq.gov.cn/ywdt/jrcq/202607/t20260723_15843821.html  
- **news.qq.com** (腾讯新闻) — https://news.qq.com/rain/a/20260719A043HV00

Used this grep pattern to extract Chinese text from the full HTML:
```bash
curl -sL "URL" -H "User-Agent: ..." 2>/dev/null | grep -oP '(?<=>)[^<>]*(?:彭水|救援|搜救|生命|滑坡|崩塌|失联|安置|应急)[^<>]*(?=<)' | grep -v '^\s*$' | head -40
```

### Text extraction from content-rich pages

The government site (cq.gov.cn) and science daily (stdaily.com) returned full article text visible in grep matches. The QQ News article was embedded in JSON within a `<script>` tag under `window.DATA` with the article body in HTML entities inside `originContent.text` — too complex for simple grep; use a different approach for Tencent news pages.

## What did NOT work or was unreliable

### Baidu mobile (m.baidu.com)
- First attempt returned some results (4 article titles via innerTitle grep)
- All subsequent attempts returned empty — rate limiting was harsher than anticipated
- Attempts with delays (2s sleep) and different query phrasing still failed

### Sogou WeChat search (weixin.sogou.com)
- Searched "彭水山体滑坡救援", "重庆彭水滑坡", "彭水+山体+崩塌", "重庆彭水+山体+救援" — ALL returned empty
- Even the `<h3>` heading grep pattern returned nothing — zero results for this disaster topic
- This is worse than the first session where at least some unrelated results appeared

### Direct news site access — thepaper.cn
- The model-hallucinated URL (thepaper.cn/newsDetail_forward_31234567) returned an entirely unrelated article about a community service event in Heilongjiang
- The actual thepaper.cn article URL from Bing was newsDetail_forward_33611203

### Sogou web search (www.sogou.com)
- Returned garbled encoding (mojibake) — curl output was in wrong encoding or the page detection failed
- Not useful as a fallback

## Comprehensive findings from Bing + direct article access

### Event details
- Date: July 17, 2026 ~9:08 AM
- Location: 彭水苗族土家族自治县汉葭街道, 乌江右岸
- Type: 山体崩塌 (rock collapse, not mudslide)
- Geology: 硬质灰岩 (hard limestone), 30m vertical drop
- Largest single boulder: 5,300 cubic meters
- Total debris: 78,000 cubic meters (after blasting/displacement)
- 10 buildings buried along river road

### Casualties (evolving)
- 8 confirmed dead (as of ~July 19)
- 10 successfully rescued
- 34 initially missing
- By July 23: "survival rate very low" for remaining missing

### Rescue operation
- Geological disaster Level I emergency response (国家I级应急响应)
- 872 rescue personnel
- 1,100+ pieces of equipment
- 13 professional rescue teams
- 150+ experts deployed nationally
- China Anneng Group (中国安能集团) leading
- 3 controlled blasting operations by July 23 to break up mega-boulders
- Rescue dog "子歇" found signs of life on July 18

### Warning and evacuation
- Resident Hou Kaiyun (侯开云) discovered rock cracks early morning (~8 AM)
- Neighbor Xu Zhaoquan (徐召全) joined the warning effort
- Knocked door-to-door, evacuated 60+ residents
- Total 1,100+ residents within 1km evacuated
- Hotels commandeered for centralized shelter

### Medical response
- 33 multidisciplinary experts from 8 top-tier hospitals
- West China Hospital of Sichuan University, Peking University People's Hospital (National Trauma Medical Center) involved
- Multiple critically ill patients stabilized

### High-level response
- Xi Jinping gave important instructions
- Chongqing Party Secretary Yuan Jiajun (袁家军) directed operations
- Full geological hazard inspection across Pengshui county

### Technical challenges
- Narrow workspace (single road between mountain and river)
- Unstable slope with secondary collapse risk
- Mega-boulders requiring controlled directional blasting
- Rain and high temperatures alternating
- Radar monitoring for secondary slides
- Full-site 3D model from lidar and drone photogrammetry
