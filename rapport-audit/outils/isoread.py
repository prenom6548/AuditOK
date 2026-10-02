import subprocess,struct,sys,os
U=sys.argv[1]; OUT=sys.argv[2]
def rng(off,n):
    for i in range(5):
        r=subprocess.run(['curl','-sS','--max-time','120','-r',f'{off}-{off+n-1}',U],capture_output=True)
        if r.returncode==0 and len(r.stdout)==n: return r.stdout
    raise Exception(f'range fail {off} {n}')
pvd=rng(16*2048,2048); assert pvd[1:6]==b'CD001',pvd[:8]
print('VolumeID:',pvd[40:72].decode(errors='replace').strip(),'| created:',pvd[813:830].decode(errors='replace'))
root=pvd[156:156+34]; ext=struct.unpack('<I',root[2:6])[0]; ln=struct.unpack('<I',root[10:14])[0]
def readdir(ext,ln):
    data=rng(ext*2048,ln); out=[];i=0
    while i<len(data):
        l=data[i]
        if l==0: i=((i//2048)+1)*2048; continue
        rec=data[i:i+l]; e=struct.unpack('<I',rec[2:6])[0]; s=struct.unpack('<I',rec[10:14])[0]; fl=rec[25]; nl=rec[32]; name=rec[33:33+nl]
        # Rock Ridge NM
        su=rec[33+nl+((nl+1)%2):]; j=0; rr=None
        while j+4<=len(su):
            sig=su[j:j+2]; sl=su[j+2]
            if sl==0: break
            if sig==b'NM': rr=(rr or b'')+su[j+5:j+sl]
            j+=sl
        nm=(rr.decode(errors='replace') if rr else name.decode(errors='replace').split(';')[0])
        if name not in (b'\x00',b'\x01'): out.append((nm,e,s,bool(fl&2)))
        i+=l
    return out
def walk(path,ext,ln,depth,want):
    for nm,e,s,d in readdir(ext,ln):
        p=path+'/'+nm
        print(('D ' if d else 'F ')+f'{s:>12} {p}')
        if d and depth>0: walk(p,e,s,depth-1,want)
        if (not d) and any(p.endswith(w) for w in want) and s<50_000_000:
            open(os.path.join(OUT,p.strip('/').replace('/','__')),'wb').write(rng(e*2048,s))
walk('',ext,ln,1,['/.kylin-post-actions','/.disk/base.commit','/.disk/kylin-build','genisoimage_params.txt','/.disk/cd_type','.manifest','filesystem.size','/.disk/info','md5sum.txt','SHA256SUMS','.list','filesystem.manifest-remove','.squashfs.gpg','/.disk/casper-uuid-generic'])
