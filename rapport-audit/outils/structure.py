# Extraction structurelle (Phase 0.2) : aucune lecture semantique du code.
import os,re,json,sys,subprocess,collections
LANG={'.c':'C','.h':'C/C++ hdr','.cpp':'C++','.cc':'C++','.cxx':'C++','.hpp':'C++','.py':'Python','.js':'JavaScript','.mjs':'JavaScript','.ts':'TypeScript','.tsx':'TypeScript','.go':'Go','.rs':'Rust','.java':'Java','.kt':'Kotlin','.sh':'Shell','.bash':'Shell','.qml':'QML','.ui':'Qt UI','.pl':'Perl','.rb':'Ruby','.lua':'Lua','.cs':'C#','.php':'PHP','.vue':'Vue','.s':'Asm','.S':'Asm','.proto':'Protobuf','.json':'JSON','.xml':'XML','.yaml':'YAML','.yml':'YAML','.cmake':'CMake','.pro':'qmake','.pri':'qmake','.ts~':'','.html':'HTML','.css':'CSS'}
EP=[ # (categorie, regex sur le chemin)
 ('systemd-unit',r'\.(service|socket|timer|path|mount|target)(\.in)?$'),
 ('dbus-service',r'dbus-1/(system-)?services/.*\.service|\.service\.in$|dbus.*\.service$'),
 ('dbus-policy',r'(dbus-1/system\.d/|/system\.d/).*\.conf$|dbus.*\.conf$'),
 ('polkit',r'\.policy(\.in)?$|polkit-1/rules\.d/|\.pkla$'),
 ('cron',r'(cron\.(d|daily|hourly|weekly|monthly)/|crontab|\.cron(\.d)?$)'),
 ('udev',r'(udev/rules\.d/|\.rules$)'),
 ('autostart',r'(xdg/autostart/|/autostart/.*\.desktop$|autostart.*\.desktop$)'),
 ('maintainer-script',r'debian/([^/]+\.)?(preinst|postinst|prerm|postrm|config|triggers)$'),
 ('apt-hook',r'apt\.conf\.d/|apt/apt\.conf|sources\.list|preferences\.d/|trusted\.gpg'),
 ('ld-preload',r'ld\.so\.preload|ld\.so\.conf\.d/'),
 ('profile',r'profile\.d/|/etc/bash\.bashrc|environment\.d/'),
 ('sudoers',r'sudoers'),
 ('pam',r'pam\.d/|pam-configs/|pam_.*\.c$'),
 ('kernel-module',r'dkms\.conf$|Kbuild$|modprobe\.d/|modules-load\.d/|\.ko$'),
 ('sysctl-tmpfiles',r'sysctl\.d/|tmpfiles\.d/|sysusers\.d/'),
 ('nm-dispatcher',r'NetworkManager/dispatcher\.d/'),
 ('selinux-apparmor',r'apparmor\.d/|\.te$'),
 ('initramfs-grub',r'initramfs-tools/|grub\.d/'),
 ('desktop-file',r'\.desktop(\.in)?$'),
 ('vsix-extension',r'\.vsix$|/extensions/[^/]+/package\.json$'),
 ('debian-install-lists',r'debian/[^/]*\.(install|links|dirs|conffiles)$'),
 ('debian-rules',r'debian/rules$'),
 ('debian-control',r'debian/control$'),
]
EPR=[(c,re.compile(r)) for c,r in EP]
LISTEN=re.compile(rb'(QTcpServer|QLocalServer|QWebSocketServer|QUdpSocket::bind|\bbind\s*\([^;)]{0,80}sockaddr|(?<![\w.:])listen\s*\(\s*\w+\s*,\s*\w+\s*\)|socketserver|http\.server|HTTPServer|\.listen\(|net\.Listen|grpc::ServerBuilder|ServerBuilder|websockets\.serve|Flask\(|FastAPI\(|uvicorn\.run|createServer\()')
MAIN=re.compile(rb'(\bint\s+main\s*\(|if\s+__name__\s*==\s*[\'"]__main__|^func main\(\)|fn main\(\))',re.M)
SKIPDIR={'.git','.pc'}
def scan(root):
    st=dict(largest=[],root=root,files=0,bytes=0,lang=collections.Counter(),entry=collections.defaultdict(list),listen=[],mains=[],blobs=[],symlinks_out=[])
    for dp,dn,fn in os.walk(root):
        dn[:]=[d for d in dn if d not in SKIPDIR]
        for f in fn:
            p=os.path.join(dp,f); rel=os.path.relpath(p,root)
            if os.path.islink(p):
                t=os.readlink(p)
                if t.startswith('/') or '..' in t: st['symlinks_out'].append(rel+' -> '+t)
                continue
            try: sz=os.path.getsize(p)
            except: continue
            st['files']+=1; st['bytes']+=sz
            st['largest'].append((rel,sz)); st['largest']=sorted(st['largest'],key=lambda x:-x[1])[:8]
            ext=os.path.splitext(f)[1]
            st['lang'][LANG.get(ext,LANG.get(ext.lower(),'other'))]+=sz
            for c,r in EPR:
                if r.search(rel): st['entry'][c].append(rel)
            with open(p,'rb') as fh: head=fh.read(4096)
            if sz<131072 and ext in ('.conf','.xml','.service','.in','.policy','.rules','.timer','.socket'):
                body=open(p,'rb').read()
                if b'<busconfig' in body: st['entry']['dbus-policy(contenu)'].append(rel)
                if b'[D-BUS Service]' in body: st['entry']['dbus-service(contenu)'].append(rel)
                if b'<policyconfig' in body or b'polkit.addRule' in body: st['entry']['polkit(contenu)'].append(rel)
                if b'[Service]' in body and b'ExecStart' in body: st['entry']['systemd-service(contenu)'].append(rel)
                if b'[Timer]' in body: st['entry']['systemd-timer(contenu)'].append(rel)
                if b'[Socket]' in body: st['entry']['systemd-socket(contenu)'].append(rel)
            magic=head[:4]
            isbin = head.startswith(b'SQLite format 3') or magic.startswith(b'\x7fELF') or magic[:2]==b'MZ' or magic==b'!<ar' or head[:8]==b'!<arch>\n' or magic==b'dex\n' or magic[:2]==b'PK' and ext in ('.jar','.apk','.aar','.vsix','.whl','.zip','.xpi') or ext in ('.so','.a','.o','.ko','.bin','.img','.pyc','.class','.wasm','.dll','.exe','.apk','.jar','.onnx','.gguf','.pt','.pth','.pb','.tflite','.dat','.node','.dex','.oat','.vdex')
            if isbin and not ext in ('.png','.jpg','.svg','.ico','.gif','.ttf','.otf','.woff','.woff2','.mo','.qm','.wav','.ogg','.mp3','.mp4'):
                st['blobs'].append((rel,sz,head[:4].hex()))
            if sz<2_000_000 and ext in ('.c','.cpp','.cc','.cxx','.py','.go','.rs','.js','.ts','.java','.h','.hpp'):
                data=open(p,'rb').read()
                if MAIN.search(data): st['mains'].append(rel)
                m=LISTEN.search(data)
                if m: st['listen'].append((rel,m.group(0).decode(errors='replace')))
    st['lang']=dict(st['lang'].most_common(8)); st['entry']={k:v[:60]+([f'...(+{len(v)-60})'] if len(v)>60 else []) for k,v in st['entry'].items()}
    st['n_entry']={k:len(v) for k,v in st['entry'].items()}
    st['listen']=st['listen'][:40]; st['mains']=st['mains'][:40]; st['blob_count']=len(st['blobs']); st['blob_bytes']=sum(b[1] for b in st['blobs']); st['blobs']=sorted(st['blobs'],key=lambda b:-b[1])[:40]
    return st
if __name__=='__main__':
    out={}
    for root in sys.argv[2:]: out[os.path.basename(root.rstrip('/'))]=scan(root)
    json.dump(out,open(sys.argv[1],'w'),indent=1,default=list)
