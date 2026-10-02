import json,os,collections
S=__import__('os').environ.get('AUDIT_WORK','travail')
SRCDIR=__import__('os').environ.get('AUDIT_SRC','sources')
C=json.load(open(S+'/candidates.json')); master={(r['paquet_source']):r for r in json.load(open(S+'/master.json')) if r['depot'] in ('build','ppa-anything3.0')}
st=json.load(open(S+'/struct_all.json')); forge=json.load(open(S+'/forge_status.json')); iso=json.load(open(S+'/apt2/iso_map.json'))
isob=collections.defaultdict(list)
for b,v in iso.items():
    if v['src']: isob[v['src']].append(f"{b}={v['ver']}")
BATCH={
 'B01-IA-coeur':['kylin-ai-sdk','libkylin-ai-system-private','libkysdk-ai-private','kylin-ai-runtime','kylin-ai-subsystem','kylin-ai-subsystem-plugin','kylin-ai-subsystem-modelconfig','kylin-ai-business-framework-service','libkyai-business-framework','kylin-ai-cloud-model','kylin-ai-engine-plugins','libkylin-ai-engine-dev','libkylin-ai-base','libkylin-ai-proto','libkylin-ai-default-key','kylin-aiassistant','kylin-ai-data-management-service','libkyai-data-management-client','kylin-ai-document-service','kylin-ai-document-qa-service','kylin-ai-vector-engine','libkysdk-vector-engine-client','kysdk-ai-common','libkysdk-mcp','libkysdk-advanced-ai','libkyai-assistant','libkyai-config','kylin-cua'],
 'B02-IA-moteurs-agents':[c['name'] for c in C if c['group']=='G01-IA' and c['tier']=='B']+['libkysdk-genai-nlp','libkysdk-genai-vision','libkysdk-coreai-vision','libkysdk-coreai-speech','kylin-coreai-embedding','kylin-ai-parser-extension','kylin-asrassistant','llm-backend','kytensor','kytensor-llm','onnxruntime-backend','ukui-menu-extensions','ukui-intelligent-data-manager','kylin-ai-abstract-models','kylin-agent','kylin-agent-armor','agent-runtime','computer-use-agent','kylin-voice-input','ukui-voice-wing','kylin-ai-model-manager','kylin-model-management-tools','model-management-tool'],
 'B03-kylin-code':['kylin-code','extensions-repo'],
 'B04-boutique-updateur':[c['name'] for c in C if c['group'] in ('G03-boutique-installation-apps','G04-updateur')],
 'B05-installateur-ostree-P3':[c['name'] for c in C if c['group'] in ('G05-installateur-iso','G10-ostree-immuable') or c['prio']=='P3'],
 'B06-android':[c['name'] for c in C if c['group']=='G06-android-kmre-kare'],
 'B07-gel-telemetrie':[c['name'] for c in C if c['group']=='G07-gel-gradue-processus']+['kylin-daq','libkysdk-diagnosetest','libkysdk-base','kydoctor','kylin-os-manager','kylin-driver-manager-service','kylin-sso-client','kylin-mfa','openkylin-token-center','kylin-wget','kylin-bot','kylin-botshell','platform-security-patrol','kylin-device-daemon','kylin-connectivity'],
 'B08-kysdk-biometrie':['kba','libboundscheck','libkysdk-system','libkysdk-applications','libkysdk-desktop','ukui-clipboard','ukui-search','libkylin-chkname','biometric-authentication','ukui-biometric-auth','ukui-biometric-manager','biometric-driver-fido','biometric-driver-wechat','smart-interconnection-backend','smart-interconnection-frontend'],
 'B09-ukui-1':['kylin-nm-plugin','libkylin-nm-base','ukui-session-manager','ukui-settings-daemon','ukui-control-center','ukui-screensaver','ukui-greeter','ukui-panel','ukui-menu','peony','peony-extensions','kylin-nm','ukui-notification','ukui-notification-daemon','kylin-wayland-compositor','ukui-kwin','ukui-sidebar'],
 'B10-ukui-2':[c['name'] for c in C if c['group']=='G09-ukui-coeur' and c['name'] not in ('kylin-nm-plugin','libkylin-nm-base','ukui-session-manager','ukui-settings-daemon','ukui-control-center','ukui-screensaver','ukui-greeter','ukui-panel','ukui-menu','peony','peony-extensions','kylin-nm','ukui-notification','ukui-notification-daemon','kylin-wayland-compositor','ukui-kwin','ukui-sidebar')],
 'B14-apps-kylin':[c['name'] for c in C if c['group']=='G11-applications-kylin'],
 'B11-P2A-1':['linux','systemd','glibc','grub2','grub2-signed','sudo','pam','policykit-1','shadow','lightdm','openssh','whoopsie','whoopsie-preferences','apport'],
 'B12-P2A-2':['openssl','openssl1','gnutls28','nss','libgcrypt20','ca-certificates','gnupg2','glib-networking','curl','wget','network-manager','wpa','firefox','apt','dpkg','python-apt','aptdaemon','libqapt','update-manager','python-pip','ostree'],
 'B13-P2B':[c['name'] for c in C if c['prio']=='P2' and c['tier']=='B'],
}
seen=set(); out={}
for b,names in BATCH.items():
    L=[]
    for n in dict.fromkeys(names):
        seen.add(n)
        m=master.get(n,{}); s_arch=st.get('archive/'+n); s_git=st.get('gitee/'+n)
        def trim(s):
            if not s: return None
            d={k:s.get(k) for k in ['files','bytes','lang','n_entry','entry','listen','mains','blob_count','blob_bytes','blobs','largest']}; d['symlinks_out_count']=len(s.get('symlinks_out',[])); d['symlinks_out']=s.get('symlinks_out',[])[:15]; return d
        L.append(dict(nom=n,chemin_archive=(SRCDIR+'/archive/'+n if s_arch else None),chemin_gitee=(SRCDIR+'/gitee/'+n if s_git else None),
           meta={k:m.get(k) for k in ['depot','composant','version','format','dans_debian','dans_ubuntu','origine_kylin','orig_vs_debian_ubuntu','taille_source_octets','source_coquille_vide','vcs_git','mainteneur','priorite','groupe','niveau']},
           installe_par_defaut_iso=isob.get(n,[]),forges={k:forge.get(n,{}).get(k) for k in ['gitee','github','rel','gitee_heads']},
           struct_archive=trim(s_arch),struct_gitee=trim(s_git)))
    out[b]=L
missing=[c['name'] for c in C if c['name'] not in seen and c['tier']!='X']
os.makedirs(S+'/digest',exist_ok=True)
for b,L in out.items(): json.dump(L,open(f'{S}/digest/{b}.json','w'),ensure_ascii=False,indent=1)
print({b:len(L) for b,L in out.items()}); print('candidates not in any batch:',missing)
