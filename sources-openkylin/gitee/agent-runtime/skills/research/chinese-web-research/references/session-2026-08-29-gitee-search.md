# Gitee Search Techniques

## Problem
Searching Gitee.com for projects cannot be done reliably via curl or API calls:
- Gitee search pages (so.gitee.com, gitee.com/search) are SPA applications requiring JavaScript rendering
- Gitee API v5 (`/api/v5/search/projects`, `/api/v5/repos`) returns 404 without authentication token
- curl-based scraping returns empty or 404 HTML pages

## Solution
Use kylin-cua to open a browser and perform the search interactively.

### Workflow (2-step approach)
1. **Step 1**: Open browser, navigate to `https://gitee.com/search?q=YOUR_QUERY&type=project`, wait for page load
2. **Step 2**: Extract project information (name, stars, description, last update time) from the loaded page

### Important notes
- Task splitting is essential — do not combine navigation and extraction in a single kylin-cua call
- The `record_info` action in kylin-cua does NOT write files to disk — it only records in memory
- After kylin-cua returns results, use `write_file` to save information to the desired location
- Gitee search results can be sorted by stars (Star数) using the dropdown on the search page

### Example task descriptions
- Step 1: `打开浏览器访问 https://gitee.com/search?q=agent&type=project 等待页面加载完成`
- Step 2: `查看当前浏览器页面，找到按star数排序的前N个项目。记录每个项目的：项目名称、star数、简介、最近更新时间。然后将这些信息告诉我。`

## Session Date
2026-08-29
