---
name: audio
description: Device volume and mute controls
version: 0.0.1
author: ukui
tags:
  - volume
---

# Audio conversation tests

## Test flow

### 1. Get output volume
> What's the current volume

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.getSinkVolume
```

### 2. Set output volume
> Set volume to 60

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.setSinkVolume 60
```

### 3. Mute
> Mute

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.setSinkMute true
```

### 4. Unmute
> Unmute

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.setSinkMute false
```

### 5. Get mute state
> Is it muted right now

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.getSinkMute
```

### 6. Get output device
> Which output device is in use

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.getSinkDevice
```

### 7. Get microphone volume
> What's the microphone volume

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.getSourceVolume
```

### 8. Set microphone volume
> Set microphone volume to 50

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.setSourceVolume 50
```

### 9. Mute microphone
> Mute the microphone

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.setSourceMute true
```

### 10. Switch output device
> Switch to speaker output

```
gdbus call --session --dest org.ukui.Framework \
  --object-path /org/ukui/Framework/Devices/Audio \
  --method org.ukui.Framework.Devices.Audio.setSinkDevice \
  "alsa_output.pci-0000_00_1f.3.analog-stereo" "analog-output-speaker"
```

---

## Sample conversation

**You**: Set volume to 40
**Bot**: ✅ Set to 40

**You**: Mute
**Bot**: ✅ Muted

**You**: Unmute and set volume to 60
**Bot**: ✅ Unmuted, volume set to 60

**You**: Mute the microphone
**Bot**: ✅ Microphone muted
