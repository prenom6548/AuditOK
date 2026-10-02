import json,os,re,collections
S=__import__('os').environ.get('AUDIT_WORK','travail')
ep=json.load(open(S+'/contents_ep.json')); BIN=json.load(open(S+'/apt2/bin_all.json')); iso=json.load(open(S+'/iso/manifest.json'))
C={c['name']:c for c in json.load(open(S+'/candidates.json'))}
# binonly lists
PAT={'unit-systeme':r'/(usr/)?lib/systemd/system/[^/]+\.service$|/etc/systemd/system/[^/]+\.service$|/opt/system/lib/systemd/system/[^/]+\.service$','unit-utilisateur':r'/systemd/user/[^/]+\.service$','timer':r'/systemd/(system|user)/[^/]+\.timer$','dbus-systeme-activable':r'dbus-1/system-services/[^/]+\.service$','dbus-politique-systeme':r'dbus-1/system\.d/[^/]+\.conf$','polkit-action':r'polkit-1/actions/[^/]+\.policy$','pam':r'pam\.d/[^/]+$|pam-configs/[^/]+$|pam_[^/]+\.so$','sudoers':r'sudoers\.d/[^/]+$','apt-conf':r'/etc/apt/[^/]+\.d/[^/]+$','cron':r'/etc/cron\.[^/]+/[^/]+$','udev':r'udev/rules\.d/[^/]+$','autostart-xdg':r'xdg/autostart/[^/]+\.desktop$','ld-preload':r'ld\.so\.(preload|conf\.d/[^/]+)$','module-noyau':r'\.ko(\.[a-z]+)?$|/dkms\.conf$','initramfs':r'initramfs-tools/(hooks|scripts/[^/]+)/[^/]+$'}
bo={}
D=S+'/binonly/out'
for b in os.listdir(D):
    f=os.path.join(D,b,'fichiers.txt'); res=collections.defaultdict(list)
    for l in open(f,errors='replace'):
        x=l.split()
        if not x: continue
        path=x[5] if len(x)>5 else x[-1]
        path=path.lstrip('.')
        for c,r in PAT.items():
            if re.search(r,path): res[c].append(path)
    bo[b]=res
srcbins=collections.defaultdict(set)
for b,L in BIN.items():
    for x in L:
        if x['repo'] in ('build','software','ppa-anything3.0'): srcbins[x['src']].add(b)
out={}
for n,c in C.items():
    bins=srcbins.get(n,set())|({n} if n in bo else set())
    agg=collections.defaultdict(set); inst=collections.defaultdict(set)
    for b in bins:
        src=bo.get(b) or ep.get(b) or {}
        for cat,paths in src.items():
            if cat not in PAT and cat not in ('unit-systeme','unit-utilisateur','dbus-systeme-activable','dbus-politique-systeme','polkit-action','pam','sudoers','apt-conf','cron','udev','autostart-xdg','ld-preload','module-noyau','initramfs','timer','socket','systemd-generateur','nm-dispatcher','modprobe','profile','grub','dbus-session-activable','polkit-regle'): continue
            for p in paths:
                if p.endswith('/'): continue
                agg[cat].add(p)
                if b in iso: inst[cat].add(p)
    out[n]={'tous':{k:sorted(v) for k,v in agg.items()},'installes_iso':{k:sorted(v) for k,v in inst.items()},'binaires':sorted(bins),'source_index':'Contents(main)+deb(pty/software)'}
json.dump(out,open(S+'/surface.json','w'),ensure_ascii=False,indent=0)
tot=collections.Counter(); toti=collections.Counter()
for n,v in out.items():
    for k,x in v['tous'].items(): tot[k]+=len(x)
    for k,x in v['installes_iso'].items(): toti[k]+=len(x)
print('tous',dict(tot)); print('installes',dict(toti))
