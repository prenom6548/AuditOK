import json,re,lzma,gzip,glob,collections,csv
S=__import__('os').environ.get('AUDIT_WORK','travail')
SRC=json.load(open(S+'/apt2/src_all.json')); BIN=json.load(open(S+'/apt2/bin_all.json')); iso=json.load(open(S+'/apt2/iso_map.json'))
forge=json.load(open(S+'/forge_status.json')); cand={c['name']:c for c in json.load(open(S+'/candidates.json'))}
ph={p[0]:p for p in json.load(open(S+'/placeholders.json'))}
def stanzas(fn):
    op=lzma.open if fn.endswith('xz') else gzip.open
    d=op(fn,'rt',encoding='utf-8',errors='replace').read()
    for s in d.split('\n\n'):
        if not s.strip(): continue
        f={};k=None
        for l in s.split('\n'):
            if l[:1] in ' \t' and k: f[k]+='\n'+l; continue
            if ':' in l: k,v=l.split(':',1); f[k]=v.strip()
        if 'Package' in f: yield f
def origs(f):
    out={}
    for l in f.get('Checksums-Sha256','').strip().split('\n'):
        x=l.split()
        if len(x)==3 and re.search(r'\.orig(-[A-Za-z0-9-]+)?\.tar\.[a-z0-9]+$',x[2]): out[x[2]]=x[0]
    return out
ref={}; refnames={'debian':set(),'ubuntu':set()}
for fn in sorted(glob.glob(S+'/apt/debian_*_Sources.xz')+glob.glob(S+'/apt/ubuntu_*_Sources.xz')):
    tag=fn.split('/')[-1].replace('_Sources.xz','')
    for f in stanzas(fn):
        refnames['debian' if tag.startswith('debian') else 'ubuntu'].add(f['Package'])
        for name,h in origs(f).items(): ref.setdefault(name,{})[h]=tag
isobins=collections.defaultdict(list)
for b,v in iso.items():
    if v['src']: isobins[v['src']].append(b)
kre=re.compile(r'(kylin|ukui|peony|kysdk|libky|kmre|kare|kaiming|openkylin|^ok-|youker|ubuntukylin|biometric|kydoctor|yhkylin|ostree-(maintain|coredata|extra)|token-center|kytensor|llm-backend)')
rows=[]
for key,f in SRC.items():
    repo,p=key.split('/',1)
    if repo=='ppa-anything3.0' and not (p in isobins or p in cand or kre.search(p)): continue
    files=[l.split() for l in f.get('Files','').strip().split('\n') if l.strip()]
    srcbytes=sum(int(x[1]) for x in files if not x[2].endswith('.dsc'))
    o=origs(f)
    if not o: oc='aucun-orig(natif)' if 'native' in f.get('Format','') else 'aucun-orig'
    else:
        st=[]
        for n,h in o.items():
            if n not in ref: st.append('amont-inconnu')
            elif h in ref[n]: st.append('IDENTIQUE')
            else: st.append('EMPREINTE-DIFFERENTE')
        oc='IDENTIQUE' if all(s=='IDENTIQUE' for s in st) else ('EMPREINTE-DIFFERENTE' if 'EMPREINTE-DIFFERENTE' in st else 'amont-inconnu')
    m=f.get('Maintainer','')
    kyl=bool(kre.search(p)) or 'kylinos.cn' in m or (p not in refnames['debian'] and p not in refnames['ubuntu'] and 'native' in f.get('Format',''))
    fr=forge.get(p,{})
    c=cand.get(p,{})
    pay=max([x['isize'] for b,L in BIN.items() for x in L if x['src']==p and x['repo'].startswith(repo[:5])] or [0]) if p in cand else None
    rows.append(dict(paquet_source=p,depot=repo,composant=f.get('_comp'),version=f.get('Version'),format=f.get('Format'),
        dans_debian=int(p in refnames['debian']),dans_ubuntu=int(p in refnames['ubuntu']),origine_kylin=int(kyl),
        orig_vs_debian_ubuntu=oc,taille_source_octets=srcbytes,source_coquille_vide=int(p in ph),
        binaires_installes_iso=len(isobins.get(p,[])),gitee=fr.get('gitee','non-verifie'),github=fr.get('github','non-verifie'),
        relation_gitee_github=fr.get('rel',''),tags_build_gitee=len(fr.get('build_tags',[])),
        vcs_git=f.get('Vcs-Git',''),mainteneur=m,priorite=c.get('prio',''),groupe=c.get('group',''),niveau=c.get('tier','')))
rows.sort(key=lambda r:(r['priorite'] or 'Z',r['groupe'],r['paquet_source']))
json.dump(rows,open(S+'/master.json','w'),ensure_ascii=False)
with open(S+'/master.csv','w',newline='') as fh:
    w=csv.DictWriter(fh,fieldnames=list(rows[0].keys())); w.writeheader(); w.writerows(rows)
cnt=collections.Counter
b=[r for r in rows if r['depot'] in('build','build-security')]
print('rows',len(rows),'build',len(b))
print('orig',cnt(r['orig_vs_debian_ubuntu'] for r in b))
print('kylin-origin',sum(r['origine_kylin'] for r in b),'| installed (>=1 bin in ISO):',sum(1 for r in b if r['binaires_installes_iso']))
print('kylin-origin & installed',sum(1 for r in b if r['origine_kylin'] and r['binaires_installes_iso']))
print('gitee',cnt(r['gitee'] for r in b)); print('github',cnt(r['github'] for r in b)); print('rel',cnt(r['relation_gitee_github'] for r in b))
print('placeholders',sum(r['source_coquille_vide'] for r in b))
print('DIFF-HASH among installed:',sorted(r['paquet_source'] for r in b if r['orig_vs_debian_ubuntu']=='EMPREINTE-DIFFERENTE' and r['binaires_installes_iso'])[:200])
