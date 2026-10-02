import json,collections,re
S=__import__('os').environ.get('AUDIT_WORK','travail')
M=json.load(open(S+'/wf_results_corrected.json')); pk=M['pk']
C={c['name']:c for c in json.load(open(S+'/candidates.json'))}
master={r['paquet_source']:r for r in json.load(open(S+'/master.json')) if r['depot'] in ('build','ppa-anything3.0')}
cv=json.load(open(S+'/codevol.json')); SF=json.load(open(S+'/surface.json')); cld=json.load(open(S+'/changelog_dates.json'))
import os
def gapi(n):
    f=f'{S}/giteeapi/{n}.json'
    try: d=json.load(open(f)); return (d.get('pushed_at') or '')[:10]
    except Exception: return ''
def lines(n): return max(cv.get('archive/'+n,{}).get('code_lines',0),cv.get('gitee/'+n,{}).get('code_lines',0))
def fm(x): return f"{int(x):,}".replace(',','\u202f')
def esc(s): return (s or '').replace('|','\\|').replace('\n',' ')
GN={'G01-IA':'IA (kylin-ai-*, moteurs, agents)','G02-kylin-code':'kylin-code (IDE)','G03-boutique-installation-apps':'Boutique, installation d\'applications','G04-updateur':'Updateur maison','G05-installateur-iso':'Installateur, outils ISO/sauvegarde','G06-android-kmre-kare':'Android (KMRE) et compatibilité (KARE)','G07-gel-gradue-processus':'Gestion des processus (« Graded Freeze »)','G08-telemetrie-diagnostic-identite':'Télémétrie, diagnostic, identité, sécurité','G09-ukui-coeur':'UKUI cœur et services de session','G10-ostree-immuable':'Système immuable OSTree','G11-applications-kylin':'Applications Kylin, thèmes, polices','G12-binaires-tiers-proprietaires':'Binaires tiers / propriétaires, modèles','P2-A-noyau':'P2-A noyau','P2-A-init-journal':'P2-A systemd','P2-A-crypto-tls':'P2-A crypto/TLS','P2-A-reseau-clients':'P2-A clients réseau','P2-A-paquets-maj':'P2-A paquets / mises à jour','P2-A-identite-privileges':'P2-A identité / privilèges','P2-A-libc-boot':'P2-A libc / amorçage','P2-A-rapport-erreurs':'P2-A rapports d\'erreurs','P2-B-divers':'P2-B autres forks','P3-confiance-depots':'P3 confiance / dépôts','P3-build-iso':'P3 construction / ISO'}
EFF={1:2,2:6,3:16,4:40,5:80}  # heures-auditeur indicatives par niveau d'effort
def detail():
    out=['# 01a — Points d\'entrée par composant (Phase 0.2, détail)','',
         '> Recensement **en lecture seule** des fichiers déclaratifs (unités systemd, D-Bus, polkit, scripts mainteneur, hooks APT,',
         '> modules noyau, autostart, CMake/qmake…), produit par 14 agents puis contrôlé par un agent critique.',
         '> Aucune logique applicative n\'a été jugée. Chaque point d\'entrée cite un fichier relatif à l\'arbre `archive:` (source',
         '> du paquet de l\'archive par défaut) ou `gitee:` (clone superficiel). Registres : CONSTATÉ / PROBABLE / NON COUVERT.','',
         '## Contrôle critique et corrections appliquées','',
         '> Un agent critique indépendant a relu les 14 lots : **aucun composant omis** (329/329). Il a relevé '+str(len(M['critique']['ecarts']))+' écarts',
         '> et '+str(len(M['critique']['points_entree_manquants']))+' points d\'entrée manquants, tous intégrés ci-dessous (mention « [ajout critique] » / « [correction critique] »).','',
         '**Synthèse de la critique** : '+esc(M['critique']['synthese']),'',
         '**Règles adoptées après critique** :','',
         '- Base de calcul unique des pourcentages : P1 = code propre (hors code tiers embarqué, traductions Qt, code généré) ; P2 = delta Kylin (debian/, correctifs, diff de l\'orig.tar) ; P3 = fichiers de configuration et scripts. Plages : L3 90–100 %, L2 30–60 %, L1 5–15 %, X 0 %.',
         '- Version livrée sans source correspondante = X ; un code Gitee d\'une autre version ne sert qu\'à un audit « par procuration », étiqueté comme tel.',
         '- Un squelette `dh_make` sans binaire dans l\'arbre est « absent-ou-vide » ; « reconditionnement-binaire » est réservé aux arbres contenant des binaires précompilés.',
         '- PROBABLE est réservé aux indices tirés d\'un fichier cité ; les analogies passent en NON COUVERT.',
         '- Les constats négatifs sur les écoutes réseau valent « CONSTATÉ sur les indices de l\'extracteur ; NON COUVERT au-delà » (l\'extracteur ne reconnaît ni `g_dbus_server_new_sync` ni le gRPC Python).',
         '- Les constats dépassant la Phase 0 (sémantique du code, recherche de secrets) sont des **hypothèses de Phase 1**, non des findings.','',
         '| Lot | Paquet(s) | Écart relevé | Correction |','|---|---|---|---|']+[f"| {esc(e['lot'])} | {esc(e['paquet'])} | {esc(e['ecart'])} | {esc(e['correction'])} |" for e in M['critique']['ecarts']]+['']
    for l in M['lots']:
        out+=[f"## Lot {l['lot']}",'']
        names=[n for n,p in pk.items() if p['_lot']==l['lot']]
        for n in names:
            p=pk[n]; b=p['budget']; inst=master.get(n,{}).get('binaires_installes_iso',0)
            out+=[f"### `{n}`",'',
                  f"- **Nature** : {p['nature']} — **Amont** : {esc(p['amont'])}",
                  f"- **Installé par défaut (ISO 3.0)** : {'oui ('+str(inst)+' binaire(s))' if inst else 'non'} — **Code disponible** : {esc(p['code_disponible'])}",
                  f"- **Langages / taille** : {esc(p['langages'])} — {esc(p['taille'])} — lignes de code propres mesurées : {fm(lines(n))}",
                  f"- **Privilège** : {esc(p['privilege'])}",
                  f"- **Déclencheurs** : {esc(', '.join(p['declencheurs'])) or '—'}",
                  f"- **Indices réseau** : {esc('; '.join(p['indices_reseau'])) or '—'}",
                  f"- **Blobs** : {esc(p['blobs'])}",
                  f"- **Budget proposé** : {b['profondeur']} — {b['pct_fichiers_vises']} % ({esc(b.get('base',''))}) — effort {b['effort_relatif']}/5 — {esc(b['justification'])}",'']
            if p['points_entree']:
                out+=['| Type | Fichier | Détail | Registre |','|---|---|---|---|']
                for e in p['points_entree']: out.append(f"| {esc(e['type'])} | `{esc(e['fichier'])}` | {esc(e['detail'])} | {e['registre']} |")
                out.append('')
            if p['remarques']:
                for r in p['remarques']: out.append(f"- [{r['registre']}] {esc(r['texte'])}")
                out.append('')
        if l['constats']:
            out+=[f"#### Constats transverses du lot {l['lot']}",'']
            for c in l['constats']: out.append(f"- **[{c['registre']}]** {esc(c['texte'])} — *preuve* : {esc(c['preuve'])}")
            out.append('')
    return '\n'.join(out)
def p1_section():
    rows=collections.defaultdict(list)
    for n,c in C.items():
        if c['prio']=='P1': rows[c['group']].append(n)
    out=['### 5.1 Synthèse par groupe','',
         '| Groupe | Composants | Installés par défaut | Code : complet / partiel-Gitee / aucun | Lignes de code (corpus) | Surface installée par défaut : unités système / unités utilisateur / D-Bus système / polkit / PAM / autostart / timers+cron / initramfs+ld.so | Budget L3 / L2 / L1 / X |','|---|---|---|---|---|---|---|']
    for g in sorted(rows):
        L=rows[g]; inst=sum(1 for n in L if master.get(n,{}).get('binaires_installes_iso',0))
        nat=collections.Counter(); bud=collections.Counter(); root=0; ln=0
        for n in L:
            p=pk.get(n)
            if C[n]['tier']=='X' and not p: bud['X']+=1; nat['aucun']+=1; continue
            if not p: bud['?']+=1; continue
            bud[p['budget']['profondeur']]+=1; ln+=lines(n)
            cd=p['code_disponible'].lower()
            nat['aucun' if p['nature']=='absent-ou-vide' or cd.startswith('aucun') else ('gitee' if 'gitee seulement' in cd or 'squelette' in cd else 'complet')]+=1
            if re.search(r'\broot\b',p['privilege'],re.I) and not re.search(r'aucun|sans privil',p['privilege'],re.I): root+=1
        sf=collections.Counter()
        for n in L:
            for k,v in SF.get(n,{}).get('installes_iso',{}).items(): sf[k]+=len(v)
        out.append(f"| {GN.get(g,g)} | {len(L)} | {inst} | {nat['complet']} / {nat['gitee']} / {nat['aucun']} | {fm(ln)} | {sf['unit-systeme']} / {sf['unit-utilisateur']} / {sf['dbus-systeme-activable']} / {sf['polkit-action']} / {sf['pam']} / {sf['autostart-xdg']} / {sf['timer']+sf['cron']} / {sf['initramfs']+sf['ld-preload']} | {bud['L3']} / {bud['L2']} / {bud['L1']} / {bud['X']}{(' (+'+str(bud['?'])+' non recensés)') if bud['?'] else ''} |")
    return '\n'.join(out)
def matrix():
    out=['| Priorité / groupe | Composants | Lignes de code propres | L3 | L2 | L1 | X | Effort indicatif (h-auditeur) | Couverture prévue (% des lignes propres lues) |','|---|---|---|---|---|---|---|---|---|']
    tot=collections.Counter(); groups=collections.defaultdict(list)
    for n,c in C.items(): groups[(c['prio'],c['group'])].append(n)
    for (pr,g) in sorted(groups):
        L=groups[(pr,g)]; b=collections.Counter(); ln=0; lread=0; eff=0
        for n in L:
            p=pk.get(n)
            if not p:
                b['X' if C[n]['tier']=='X' else '?']+=1; continue
            bb=p['budget']; b[bb['profondeur']]+=1; l=lines(n); ln+=l; lread+=l*min(100,bb['pct_fichiers_vises'])/100; eff+=EFF.get(bb['effort_relatif'],0)
        cov=(100*lread/ln) if ln else 0
        tot['n']+=len(L); tot['eff']+=eff
        if pr=='P1': tot['ln']+=ln; tot['lr']+=lread
        for k in 'L3 L2 L1 X ?'.split(): tot[k]+=b[k]
        out.append(f"| {pr} — {GN.get(g,g)} | {len(L)} | {fm(ln) if pr=='P1' else ('delta (voir §6)' if pr=='P2' else '—')} | {b['L3']} | {b['L2']} | {b['L1']} | {b['X']} | {eff} | {(f'{cov:.0f} %') if pr=='P1' else '—'} |")
    out.append(f"| **Total** | **{tot['n']}** | **{fm(tot['ln'])}** (P1) | {tot['L3']} | {tot['L2']} | {tot['L1']} | {tot['X']} | **{tot['eff']}** | **{(100*tot['lr']/tot['ln']) if tot['ln'] else 0:.0f} %** (P1) |")
    return '\n'.join(out),tot
if __name__=='__main__':
    open(S+'/01a.md','w').write(detail()); open(S+'/p1.md','w').write(p1_section()); m,t=matrix(); open(S+'/matrix.md','w').write(m)
    print(p1_section()); print(); print(m)
