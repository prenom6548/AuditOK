import json,subprocess,os,hashlib,sys,time,shutil
S=__import__('os').environ.get('AUDIT_WORK','travail')
R=__import__('os').environ.get('AUDIT_SRC','sources')
C=json.load(open(S+'/candidates.json')); SRC=json.load(open(S+'/apt2/src_all.json'))
BASE={'build':'http://archive.build.openkylin.top/openkylin/','ppa-anything3.0':'http://ppa.build.openkylin.top/kylinsoft/anything3.0/openkylin/'}
log=open(S+'/acquire.log','a')
def sha(f): 
    h=hashlib.sha256()
    with open(f,'rb') as fh:
        for b in iter(lambda: fh.read(1<<20),b''): h.update(b)
    return h.hexdigest()
def dl(url,dst,want):
    for i in range(6):
        subprocess.run(['curl','-sS','--max-time','1800','-o',dst,url],capture_output=True)
        if os.path.exists(dst) and sha(dst)==want: return True
        time.sleep(min(2**i,30))
    return False
def work(c):
    n=c['name']
    if c['tier']=='X': return
    key='build/'+n if 'build/'+n in SRC else ('ppa-anything3.0/'+n if 'ppa-anything3.0/'+n in SRC else None)
    if not key: print(n,'NO-SOURCE-PKG',file=log,flush=True); return
    f=SRC[key]; repo=key.split('/')[0]
    files=[l.split() for l in f['Checksums-Sha256'].strip().split('\n') if len(l.split())==3]
    native='native' in f.get('Format','')
    total=sum(int(x[1]) for x in files)
    if c['prio']=='P2':
        # P2 : .dsc + debian.tar ; natif seulement si < 30 Mo
        if native and total>30e6: files=[x for x in files if x[2].endswith('.dsc')]
        elif not native: files=[x for x in files if x[2].endswith('.dsc') or '.debian.tar' in x[2] or x[2].endswith('.diff.gz')]
    d=os.path.join(R,'dl',repo,n); os.makedirs(d,exist_ok=True)
    ok=True
    for h,sz,fn in files:
        dst=os.path.join(d,fn)
        if os.path.exists(dst) and sha(dst)==h: continue
        if not dl(BASE[repo]+f['Directory']+'/'+fn,dst,h): ok=False; print(n,'DL-FAIL',fn,file=log,flush=True)
    status='DL-OK' if ok else 'DL-PARTIAL'
    # extraction (P1/P3 completes, P2 natifs complets)
    dsc=[x[2] for x in files if x[2].endswith('.dsc')][0]
    have_all=all(os.path.exists(os.path.join(d,x[2])) for x in [l.split() for l in f['Checksums-Sha256'].strip().split('\n') if len(l.split())==3])
    out=os.path.join(R,'archive',n)
    if ok and have_all and not os.path.exists(out):
        r=subprocess.run(['dpkg-source','--no-check','--no-copy','-x',os.path.join(d,dsc),out],capture_output=True,text=True)
        status+=' EXTRACT-OK' if r.returncode==0 else ' EXTRACT-FAIL:'+r.stderr[-200:].replace('\n',' ')
    elif c['prio']=='P2' and not native:
        # extraire seulement debian/
        dt=[x[2] for x in files if '.debian.tar' in x[2]]
        if dt and not os.path.exists(out):
            os.makedirs(out,exist_ok=True); subprocess.run(['tar','-xf',os.path.join(d,dt[0]),'-C',out]); status+=' DEBIAN-ONLY'
    print(n,repo,f['Version'],status,file=log,flush=True)
from concurrent.futures import ThreadPoolExecutor
C.sort(key=lambda c:c.get('src_bytes',0))
with ThreadPoolExecutor(6) as ex: list(ex.map(work,C))
print('ACQUIRE-DONE',file=log,flush=True)
