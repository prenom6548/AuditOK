import os,json,collections
S=__import__('os').environ.get('AUDIT_WORK','travail')
CODE={'.c','.h','.cpp','.cc','.cxx','.hpp','.hh','.py','.js','.mjs','.cjs','.ts','.tsx','.go','.rs','.java','.kt','.sh','.bash','.qml','.pl','.pm','.rb','.lua','.cs','.php','.vue','.s','.S','.proto','.m','.mm','.swift','.zig','.ui'}
VENDOR=('third-party/','third_party/','3rdparty/','vendor/','node_modules/','external/','extern/','deps/','.pc/')
out={}
for base in ['archive','gitee']:
    root=__import__('os').environ.get('AUDIT_SRC','sources')+'/'+base
    for n in sorted(os.listdir(root)):
        r=os.path.join(root,n); st=collections.Counter()
        for dp,dn,fn in os.walk(r):
            dn[:]=[d for d in dn if d!='.git']
            rel=os.path.relpath(dp,r)+'/'
            for f in fn:
                p=os.path.join(dp,f)
                if os.path.islink(p): continue
                ext=os.path.splitext(f)[1]
                if ext in CODE:
                    try: sz=os.path.getsize(p)
                    except: continue
                    if ext=='.ts':
                        try:
                            h=open(p,'rb').read(512)
                            if b'<?xml' in h or b'<!DOCTYPE TS' in h or b'<TS ' in h: st['qt_translation_files']+=1; continue
                        except Exception: pass
                    if ext=='.ui': st['qt_ui_files']+=1; continue
                    if any(v in rel for v in VENDOR): st['vendor_files']+=1; st['vendor_bytes']+=sz
                    elif rel.startswith('debian/') : st['debian_files']+=1; st['debian_bytes']+=sz
                    else:
                        st['code_files']+=1; st['code_bytes']+=sz
                        try: st['code_lines']+=sum(1 for _ in open(p,'rb'))
                        except: pass
        out[base+'/'+n]=dict(st)
json.dump(out,open(S+'/codevol.json','w'))
tot=collections.Counter()
for k,v in out.items(): tot.update(v)
print(dict(tot))
