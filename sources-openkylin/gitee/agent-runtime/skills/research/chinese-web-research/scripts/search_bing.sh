#!/bin/bash
UA="Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
curl -sL "https://cn.bing.com/search?q=重庆彭水山体滑坡救援最新情况&count=20" \
  -H "User-Agent: $UA" \
  -H "Accept-Language: zh-CN,zh;q=0.9" \
  2>/dev/null | sed 's/<[^>]*>//g' | grep -v '^$' | grep -i "彭水\|山体\|崩塌\|救援\|失联\|重庆\|滑坡" | head -40
