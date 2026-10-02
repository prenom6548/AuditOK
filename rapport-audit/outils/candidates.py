import json
G={}
def add(group,prio,tier,names):
    for n in names.split():
        G.setdefault(n,dict(name=n,group=group,prio=prio,tier=tier))
# ---------------- P1 : composants propres a Kylin ----------------
add('G01-IA','P1','A','kylin-ai-sdk libkylin-ai-system-private kylin-ai-runtime kylin-ai-subsystem kylin-ai-subsystem-plugin kylin-ai-subsystem-modelconfig kylin-ai-business-framework-service libkyai-business-framework kylin-ai-cloud-model kylin-ai-engine-plugins libkylin-ai-engine-dev libkylin-ai-base libkylin-ai-proto libkylin-ai-default-key kylin-ai-model-manager kylin-model-management-tools model-management-tool kylin-aiassistant kylin-ai-data-management-service libkyai-data-management-client kylin-ai-document-service kylin-ai-document-qa-service kylin-ai-vector-engine libkysdk-vector-engine-client kysdk-ai-common libkysdk-genai-nlp libkysdk-genai-vision libkysdk-coreai-vision libkysdk-coreai-speech kylin-coreai-embedding libkyai-assistant libkyai-config kylin-ai-parser-extension kylin-asrassistant llm-backend kytensor kytensor-llm onnxruntime-backend ukui-menu-extensions ukui-intelligent-data-manager kylin-ai-abstract-models kylin-agent kylin-agent-armor agent-runtime computer-use-agent kylin-voice-input')
add('G01-IA','P1','B','kylin-deepseek-ai-engine-plugin kylin-deepseek-nlp-engine kylin-qwen-ai-engine-plugin kylin-qwen-nlp-engine kylin-sensetime-ai-engine-plugin kylin-sensetime-nlp-engine kylin-baidu-ai-engine-plugin kylin-baidu-nlp-engine kylin-baidu-speech-engine kylin-baidu-vision-engine kylin-xunfei-ai-engine-plugin kylin-xunfei-nlp-engine kylin-xunfei-speech-engine kylin-xunfei-vision-engine kylin-baichuan-ai-engine-plugin kylin-baichuan-nlp-engine kylin-custom-ai-engine-plugin kylin-custom-nlp-engine kylin-freetrial-ai-engine-plugin kylin-freetrial-nlp-engine kylin-ondevice-ai-engine-plugin kylin-ondevice-nlp-engine kylin-ondevice-speech-engine kylin-ondevice-vision-engine kylin-ondevice-embedding-engine kylin-ondevice-traditional-ai-engine-plugin kylin-ai-python-env')
add('G02-kylin-code','P1','A','kylin-code extensions-repo')
add('G03-boutique-installation-apps','P1','A','kylin-software-center kylin-installer kmre-apk-installer kylin-software-properties dpkg-wrapper aptss kylin-web-app-engine kaiming kaiming-box')
add('G03-boutique-installation-apps','P1','B','kaiming-builder kaiming-builder-gui')
add('G04-updateur','P1','A','kylin-system-updater kylin-update-frontend kylin-background-upgrade kylin-update-notify kylin-update-desktop-config revisions-manager')
add('G05-installateur-iso','P1','A','kylin-os-installer kylin-fix-boot-grub ukui-iso-customer')
add('G05-installateur-iso','P1','B','kylin-usb-creator yhkylin-backup-tools yhkylin-rockbackup-tools yhkylin-systembackup-tools')
add('G06-android-kmre-kare','P1','A','kmre kylin-kmre-daemon kylin-kmre-manager kylin-kmre-modules-dkms kylin-kmre-window kylin-kmre-display-control kylin-kmre-emugl libkylin-kmre peony-vfs-kylin-kmre kylin-kmre-image-update-x64 kare')
add('G06-android-kmre-kare','P1','B','kylin-kmre-image-data-x64 kare-v10sp1-base-x86-64 android-platform-system-core')
add('G07-gel-gradue-processus','P1','A','kylin-process-manager kylin-status-manager ukui-input-gather app-compat-dkms ukui-service-manager')
add('G08-telemetrie-diagnostic-identite','P1','A','kylin-daq libkysdk-diagnosetest libkysdk-base kydoctor kylin-os-manager kylin-driver-manager-service kylin-sso-client libkysdk-system libkysdk-applications libkysdk-desktop kylin-device-daemon kylin-connectivity platform-security-patrol kylin-bot kylin-botshell')
add('G08-telemetrie-diagnostic-identite','P1','B','ukui-clipboard ukui-search libkylin-chkname biometric-authentication ukui-biometric-auth ukui-biometric-manager biometric-driver-fido biometric-driver-wechat smart-interconnection-backend smart-interconnection-frontend')
add('G09-ukui-coeur','P1','B','ukui-session-manager ukui-settings-daemon ukui-control-center ukui-screensaver ukui-greeter ukui-panel ukui-menu peony peony-extensions kylin-nm ukui-notification ukui-notification-daemon kylin-wayland-compositor ukui-kwin ukui-sidebar ukui-power-manager ukui-media ukui-bluetooth ukui-system-monitor ukui-interface ukui-framework ukui-quick ukui-window-switch xdg-desktop-portal-ukui qt5-ukui-platformtheme kylin-virtual-keyboard youker-assistant ukui-app-widget ukui-file-metadata ukui-sni')
add('G10-ostree-immuable','P1','B','ostree-maintain-daemon ostree-coredata-protect ostree-extra-utils kylin-ostree-extra-utils')
add('G11-applications-kylin','P1','C','kylin-weather kylin-ipmsg kylin-miraclecast kylin-miraclecast-frontend kylin-camera kylin-screencap kylin-screenshot kylin-recorder kylin-printer kylin-scanner kylin-sane-virtual-backend kylin-music kylin-video kylin-photo-viewer kylin-calculator kylin-calendar kylin-font-viewer kylin-burner kylin-user-guide kylin-theme-builder ukui-clock ukui-notebook kylin-time kylincameralibs')
add('G12-binaires-tiers-proprietaires','P1','X','wps-office sogoupinyin sogou-ime-ng-data crossover openkylin-kwre-cxdepend cpis libime-cpis arise-linux-graphics-driver-dri nvidia-graphics-drivers nvidia-graphics-drivers-510 nvidia-graphics-drivers-515 nvidia-graphics-drivers-535 nvidia-graphics-drivers-550 11cudnn-local-repo 12cudnn-local-repo ai-frame-installer ccai paddlepaddle onnxruntime kylin-datamanagement-models openkylin-fonts kylin-fonts linux-4.19.246-phytium-embeded intel-microcode amd64-microcode r8168 kylin-qwen2.5-3b-gguf-model kylin-qwen2.5-3b-model kylin-cn-clip-model kylin-gte-base-model kylin-paddle-ocr-model kylin-portrait-matting-model kylin-sam-model kylin-speech-asr-model')
# ---------------- P2 : forks amont critiques ----------------
add('P2-A-noyau','P2','A','linux')
add('P2-A-init-journal','P2','A','systemd')
add('P2-A-crypto-tls','P2','A','openssl openssl1 gnutls28 nss libgcrypt20 ca-certificates gnupg2 glib-networking')
add('P2-A-reseau-clients','P2','A','curl wget network-manager openssh wpa firefox')
add('P2-A-paquets-maj','P2','A','apt dpkg python-apt aptdaemon libqapt update-manager python-pip ostree')
add('P2-A-identite-privileges','P2','A','sudo pam policykit-1 shadow lightdm')
add('P2-A-libc-boot','P2','A','glibc grub2 grub2-signed')
add('P2-A-rapport-erreurs','P2','A','whoopsie whoopsie-preferences apport')
add('P2-B-divers','P2','B','dbus cryptsetup xorg-server kwin apparmor iptables nftables packagekit util-linux kmod bubblewrap flatpak sqlcipher libsoup3 avahi bluez cups samba krb5 mokutil tpm2-tss tpm2-tools chrony strongswan xz-utils tar libarchive coreutils bash procps e2fsprogs dkms qtwebengine-opensource-src qtbase-opensource-src linux-firmware xdg-desktop-portal ubuntu-release-upgrader linglong muon bftpd')
# ---------------- P3 : construction / distribution ----------------
add('P3-confiance-depots','P3','A','openkylin-keyring ubuntukylin-keyring openkylin-archive-anything openkylin-default-settings ubuntukylin-default-settings base-files lsb')
add('P3-build-iso','P3','A','casper live-build debootstrap ubiquity openkylin-docker-images release-management server-package-list community')
json.dump(list(G.values()),open(__import__('os').environ.get('AUDIT_WORK','travail')+'/candidates.json','w'),indent=0)
import collections
print(len(G), collections.Counter((v['prio'],v['tier']) for v in G.values()))
# ---- v2 : ajouts issus de archive.build + ISO 3.0 + software + PPA ----
add('G08-telemetrie-diagnostic-identite','P1','A','kylin-mfa openkylin-token-center kylin-wget')
add('G01-IA','P1','A','libkysdk-mcp libkysdk-advanced-ai libkysdk-ai-private kylin-cua ukui-voice-wing')
add('G09-ukui-coeur','P1','B','ukui-ui-config-service ukui-shell-integration ukui-air-gesture ukui-previewhub ukui-i18n-management')
add('G06-android-kmre-kare','P1','A','liblol')
add('G11-applications-kylin','P1','C','wemeet')
json.dump(list(G.values()),open(__import__('os').environ.get('AUDIT_WORK','travail')+'/candidates.json','w'),indent=0)
print('v2',len(G))
# ---- v3 : paquets d'origine Kylin installes par defaut absents des versions precedentes ----
add('G08-telemetrie-diagnostic-identite','P1','B','kba libboundscheck')
add('G09-ukui-coeur','P1','B','kylin-nm-plugin libkylin-nm-base qt5-gesture-extensions ukui-volume-control ukui-desktop-environment kylin-ki18n time-shutdown peony-print-pictures')
add('G11-applications-kylin','P1','C','fcitx5-gb18030 fcitx5-table-ukk fcitx5-qt fcitx5-gtk ukui-globaltheme openkylin-language-pack-fr kylin-theme kylin-wallpapers fonts-ukk gb-fonts pag thorvg ukui-touch-settings-plugin')
add('G12-binaires-tiers-proprietaires','P1','X','kylin-sam-model-onnx kylin-speech-asr-model-onnx kylin-speech-tts-model kylin-speech-tts-model-onnx kylin-universal-matting-model')
json.dump(list(G.values()),open(__import__('os').environ.get('AUDIT_WORK','travail')+'/candidates.json','w'),indent=0)
print('v3',len(G))
