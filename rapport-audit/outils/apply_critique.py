import json,copy
S=__import__('os').environ.get('AUDIT_WORK','travail')
M=json.load(open(S+'/wf_results_merged.json')); pk=M['pk']; crit=json.load(open(S+'/critique.json'))
C={c['name']:c for c in json.load(open(S+'/candidates.json'))}
log=[]
def note(n,txt,reg='CONSTATÉ'):
    pk[n].setdefault('remarques',[]).append({'texte':'[correction critique] '+txt,'registre':reg}); log.append((n,txt))
def setb(n,prof,pct=None,eff=None,why=None):
    b=pk[n]['budget']; old=f"{b['profondeur']}/{b['pct_fichiers_vises']}%/e{b['effort_relatif']}"
    b['profondeur']=prof
    if pct is not None: b['pct_fichiers_vises']=pct
    if eff is not None: b['effort_relatif']=eff
    if why: b['justification']=why+' — (avant correction : '+old+')'
    log.append((n,f'budget {old} -> {prof}/{b["pct_fichiers_vises"]}%'))
# 1) reclassements nominatifs demandes par la critique
setb('avahi','L3',why='Démon root exposé au réseau (mDNS), installé par défaut : lecture exhaustive du delta Kylin (debian/, patches).')
setb('strongswan','L3',why='charon (IKE, root) installé par défaut : lecture exhaustive du delta (dont 02_disable-bypass-lan.patch, 03_systemd-service.patch, rétroportages CVE).')
setb('wpa','L3',why='wpa_supplicant root, D-Bus système, réseau, installé par défaut : delta incluant des patches D-Bus et allow-legacy-renegotiation/allow-tlsv1.')
setb('bftpd','L3',why='Serveur FTP démarré en root par le postinst (non installé par défaut) : lecture du delta.')
setb('shadow','L3',why='Binaires setuid root et 12 services PAM : lecture exhaustive du delta debian/ et patches.')
setb('kylin-code','L2',eff=4,why='Fork volumineux de code-oss 1.101.2 : X pour la version livrée (squelette dans l’archive), audit « par procuration » du delta Kylin sur Gitee (product.json, config-gitee-sync, modèles postinst/prerm/postrm, updateCheckUrl, kylin-code-server, close_telemetry.sh).')
setb('extensions-repo','L1',eff=2,why='Lecture des 9 .js, 5 .sh et des g_index.json (chaîne téléchargement-exécution) ; les 106 .vsix précompilés sont X.')
setb('grub2-signed','X',pct=0,why='Binaire EFI signé non reconstructible ; l’empaquetage (8 fichiers) sera lu en L1.')
setb('yhkylin-rockbackup-tools','L2',why='Service D-Bus système root com.kylin.backup, hooks initramfs : différentiel contre yhkylin-backup-tools (même nom de bus).')
for n in ['kylin-recorder','kylin-music','kylin-video','kylin-calculator']:
    note(n,'Lié à kysdk-datacollect : budget L1 maintenu, avec revue ciblée de tous les appels kdk_dia_* (critère « collecte »).')
# 2) regle unique : squelette dans l'archive + code Gitee d'une autre version => X pour la version livree + audit par procuration
for n in ['kylin-software-center','kylin-kmre-daemon','kylin-kmre-manager','kylin-kmre-window','kylin-kmre-display-control','kylin-kmre-emugl','libkylin-kmre','kylin-kmre-modules-dkms','kylin-code']:
    if n in pk: note(n,'Règle unique appliquée : version livrée = X (aucune source correspondante) ; le code Gitee, d’une autre version, ne peut servir qu’à un audit « par procuration », explicitement étiqueté comme tel.')
# 3) natures
for n,p in pk.items():
    cd=p['code_disponible'].lower()
    if p['nature']=='reconditionnement-binaire' and ('squelette' in cd or cd.startswith('aucun')) and not any(k in cd for k in ['binaire','.deb','elf','vsix','wheel']):
        p['nature']='absent-ou-vide'; log.append((n,'nature reconditionnement-binaire -> absent-ou-vide (squelette sans binaire)'))
for n,am in [('ukui-sni','plasma-workspace (xembed-sni-proxy), KDE'),('ukui-file-metadata','KFileMetaData, KDE'),('ukui-window-switch','KWin (switcher thumbnail_grid), KDE')]:
    pk[n]['nature']='fork-amont'; note(n,f'Reclassé fork-amont (partiel) : code dérivé de {am} ; version amont à établir en Phase 2.')
pk['smart-interconnection-frontend']['nature']='meta-ou-donnees'; note('smart-interconnection-frontend','Reclassé meta-ou-donnees (README et extrait Xilinx de 2 Ko) ; la mention d’un DaemonSet privilégié est NON COUVERT.','NON COUVERT')
note('kylin-bot','Amont non nommé ; debian/copyright déclare « Upstream-Name: kylin-aiassistant », incohérent avec le contenu (Rust/TypeScript) ; amont à établir depuis Cargo.toml en Phase 1.','CONSTATÉ')
# 4) registres : PROBABLE fondes sur connaissance generale -> NON COUVERT
for n in ['kylin-mfa','kydoctor','kylin-driver-manager-service','openkylin-token-center','kylin-background-upgrade','kylin-sane-virtual-backend']:
    if n in pk:
        for r in pk[n]['remarques']:
            if r['registre']=='PROBABLE': r['registre']='NON COUVERT'; r['texte']+=' [reclassé NON COUVERT : analogie sans indice dans un fichier]'
        log.append((n,'PROBABLE -> NON COUVERT'))
note('openkylin-token-center','Écart de version : archive 1.0.0-ok1.9, ISO 1.0.0-ok1.7 (retirée du pool, HTTP 404).')
note('policykit-1','L’ISO installe policykit-1=124-ok6.1, que la source inventoriée (127-ok3) ne produit pas : binaire sans source correspondante (NON COUVERT).','CONSTATÉ')
for n in ['glibc','grub2','linux','policykit-1']:
    note(n,'orig.tar d’empreinte différente de l’homonyme Debian/Ubuntu : vérification contre les publications amont signées préalable à la Phase 2.')
# 5) budgets : base unique
for n,p in pk.items():
    b=p['budget']; pr=C[n]['prio']; prof=b['profondeur']
    if pr=='P1':
        if prof=='L3': b['pct_fichiers_vises']=max(90,min(100,b['pct_fichiers_vises']))
        elif prof=='L2': b['pct_fichiers_vises']=max(30,min(60,b['pct_fichiers_vises']))
        elif prof=='L1': b['pct_fichiers_vises']=max(5,min(15,b['pct_fichiers_vises']))
        else: b['pct_fichiers_vises']=0
        b['base']='code propre (hors code tiers embarqué, traductions Qt et code généré)'
    else:
        b['pct_fichiers_vises']={'L3':100,'L2':50,'L1':10,'X':0}[prof]
        b['base']='delta Kylin (debian/, correctifs, diff de l’orig.tar contre l’amont)' if pr=='P2' else 'fichiers de configuration et scripts'
# 6) points d'entree manquants signales par la critique
for e in crit['points_entree_manquants']:
    n=e['paquet'].split()[-1] if e['paquet'].split()[-1] in pk else None
    if not n:
        for cand in pk:
            if cand in e['paquet']: n=cand
    if not n: log.append((e['paquet'],'PE manquant non rattaché')); continue
    pk[n]['points_entree'].append({'type':e['type']+' [ajout critique]','fichier':e['fichier'],'detail':e['detail'],'registre':'CONSTATÉ' if 'NON COUVERT' not in e['detail'] else 'NON COUVERT'})
    log.append((n,'+PE '+e['type']))
M['critique']=crit; M['journal_corrections']=log
json.dump(M,open(S+'/wf_results_corrected.json','w'),ensure_ascii=False)
print(len(log),'corrections'); 
import collections; print(collections.Counter(p['budget']['profondeur'] for p in pk.values()))
