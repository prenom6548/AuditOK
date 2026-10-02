import json
S=__import__('os').environ.get('AUDIT_WORK','travail')
T=[o for o in json.load(open(S+'/p2_table.json')) if not o.get('absent')]
out=['| Paquet | Version openKylin | Format | Homologue(s) même version amont | `orig.tar` vs Debian/Ubuntu | Correctifs `debian/patches` (dont nommés Kylin) | Entrées changelog openKylin | Installé par défaut |','|---|---|---|---|---|---|---|---|']
for o in T:
    if o['niveau']!='A': continue
    hom=', '.join(o['meme_version_amont_chez'][:2]) or '— (aucun)'
    out.append(f"| `{o['nom']}` | {o['version']} | {o['format'].replace('3.0 ','')} | {hom} | {o['orig']} | {o['patches']} ({o['patches_noms_kylin']}) | {o['entrees_changelog_ok']} | {'oui' if o['installe'] else 'non'} |")
open(S+'/p2.md','w').write('\n'.join(out)); print('\n'.join(out[:8]))
