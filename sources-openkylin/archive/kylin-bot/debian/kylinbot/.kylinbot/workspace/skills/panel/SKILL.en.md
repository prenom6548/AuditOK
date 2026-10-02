---
name: panel
description: Taskbar / panel operations
version: 0.0.1
author: ukui
tags:
  - taskbar
---

# Panel conversation tests

## Test flow

### 1. Get icon grouping
> Are taskbar icons grouped

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.GetMergeIcons
```
Values: 0=always, 1=never, 2=when the panel is full

### 2. Set icon grouping
> Always group taskbar icons

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetMergeIcon 1
```

### 3. Panel location
> Where is the taskbar

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.GetPanelLocation
```
Values: 0=bottom, 2=top, 1=left, 3=right

### 4. Set panel location
> Move the taskbar to the top

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetPanelLocation 2
```

### 5. Panel size
> Make the taskbar larger

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.GetPanelSizePolicy

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetPanelSizePolicy 2
```
Values: 0=small, 1=medium, 2=large

### 6. Auto-hide
> Auto-hide the taskbar

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.GetPanelAutoHide

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetPanelAutoHide true
```

### 7. Lock panel
> Lock the taskbar

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.GetPanelLockState

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetPanelLockState true
```

### 8. Show panel on all screens
> Show the taskbar on every screen

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.GetShowPanelOnAllScreens

gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetShowPanelOnAllScreens true
```

### 9. Show tray on all panels
> Show the system tray on every panel

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetShowSystemTrayOnAllPanels true
```

### 10. Taskbar icon workspace policy
> Show taskbar icons on all workspaces

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetTaskBarIconsShowedOn 1
```
Values: 0=current workspace only, 1=all workspaces

### 11. Task view button
> Show the task view button

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetShowTaskView true
```

### 12. Search button
> Show the search button

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/UI/Panel \
  --method org.ukui.Framework.UI.Panel.SetShowSearch true
```

---

## Sample conversation

**You**: Make the taskbar larger
**Bot**: ✅ Set to the largest size

**You**: Auto-hide the taskbar
**Bot**: ✅ Auto-hide enabled

**You**: Move the taskbar to the top
**Bot**: ✅ Moved to the top

**You**: Always group icons
**Bot**: ✅ Icons set to always group

**You**: Lock the taskbar
**Bot**: ✅ Taskbar locked

**You**: Show the taskbar on all screens
**Bot**: ✅ Multi-screen taskbar enabled

**You**: Show search and task view buttons
**Bot**: ✅ Search and task view buttons shown
