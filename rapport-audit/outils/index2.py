import gzip,lzma,json,re,collections,glob
S=__import__('os').environ.get('AUDIT_WORK','travail')
A=S+'/apt2/'
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
SRC={}   # (repo,pkg) -> stanza
for repo,pat in [('build','build_huanghe_{}_Sources.gz'),('build-security','build_huanghe-security_{}_Sources.gz')]:
    for c in ['main','pty','cross']:
        for f in stanzas(A+pat.format(c)):
            f['_repo']=repo; f['_comp']=c; SRC[(repo,f['Package'])]=f
for f in stanzas(A+'ppa_huanghe_main_Sources.gz'):
    f['_repo']='ppa-anything3.0'; f['_comp']='main'; SRC[('ppa-anything3.0',f['Package'])]=f
def isz(x):
    x=(x or '0').strip()
    m=re.match(r'([\d.]+)\s*(KB|MB|GB|kB)?',x,re.I)
    if not m: return 0
    v=float(m.group(1)); u=(m.group(2) or '').upper()
    return int(v*1024) if u=='MB' else int(v*1024*1024) if u=='GB' else int(v)
BIN=collections.defaultdict(list)  # binpkg -> list of dict(repo,comp,ver,src,srcver,isize,desc,fn,depends)
def addbins(fn,repo,comp):
    for f in stanzas(fn):
        b=f['Package']; v=f.get('Version','')
        sm=re.match(r'(\S+)(?: \((\S+)\))?',f.get('Source',b)); src=sm.group(1); sv=sm.group(2) or v
        BIN[b].append(dict(repo=repo,comp=comp,ver=v,src=src,srcver=sv,isize=isz(f.get('Installed-Size','0')),desc=f.get('Description','').split('\n')[0][:120],fn=f.get('Filename',''),maint=f.get('Maintainer',''),home=f.get('Homepage',''),dep=f.get('Depends','')[:2000],arch=f.get('Architecture','')))
for c in ['main','pty','cross']:
    addbins(A+f'build_huanghe_{c}_amd64_Packages.gz','build',c)
    addbins(A+f'build_huanghe-security_{c}_amd64_Packages.gz','build-security',c)
for c in ['main','all']: addbins(A+f'sw_huanghe_{c}_amd64_Packages.gz','software',c)
addbins(A+'ppa_huanghe_main_amd64_Packages.gz','ppa-anything3.0','main')
man=json.load(open(S+'/iso/manifest.json'))
# map ISO packages to repo providing exact version
iso_map={}
for b,v in man.items():
    hits=[x for x in BIN.get(b,[]) if x['ver']==v]
    iso_map[b]=dict(ver=v,repos=sorted(set(x['repo']+'/'+x['comp'] for x in hits)),src=(hits[0]['src'] if hits else (BIN[b][0]['src'] if b in BIN else None)),srcver=(hits[0]['srcver'] if hits else None))
json.dump({'/'.join(k):v for k,v in SRC.items()},open(S+'/apt2/src_all.json','w'))
json.dump(BIN,open(S+'/apt2/bin_all.json','w'))
json.dump(iso_map,open(S+'/apt2/iso_map.json','w'))
c=collections.Counter(tuple(v['repos']) if v['repos'] else ('NOT-FOUND-IN-ANY-REPO',) for v in iso_map.values())
print('ISO pkgs by providing repo (exact version):')
for k,n in c.most_common(): print(f'  {n:5} {k}')
nf=[b for b,v in iso_map.items() if not v['repos']]
print('NOT FOUND (exact version) :',len(nf)); print(' '.join(f"{b}={man[b]}" for b in nf)[:2500])
# source availability for ISO pkgs
srcok=collections.Counter()
missing_src=[]
for b,v in iso_map.items():
    if not v['src']: srcok['no-bin-record']+=1; continue
    found=[r for (r,p),f in SRC.items() if p==v['src'] and (v['srcver'] is None or f.get('Version')==v['srcver'])]
    if found: srcok['exact-source-'+'+'.join(sorted(set(found)))]+=1
    else: srcok['NO-EXACT-SOURCE']+=1; missing_src.append((b,v['src'],v['srcver']))
print(srcok)
json.dump(missing_src,open(S+'/apt2/iso_missing_src.json','w'))
