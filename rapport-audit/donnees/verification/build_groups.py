#!/usr/bin/env python3
"""Prépare, par thème, la liste des constats critiques/élevés à vérifier contradictoirement."""
import json, glob, os, sys

R = "/home/user/AuditOK/rapport-audit/donnees"
SRC = "/home/user/openkylin-src"
OUT = os.path.dirname(os.path.abspath(__file__))
GROUPS = {
    "G1-polkit-kaiming": ["V1-L0-kaiming", "V1-L14-approfondissement-kaiming", "P2-policykit"],
    "G2-dbus-session": ["V1-L4-processus-entrees-services", "V2-L1-ukui-session"],
    "G3-telemetrie-updater": ["V1-L1-telemetrie-daq", "V1-L5-updateur"],
    "G4-identite-os-manager": ["V2-L3-identite-telemetrie"],
    "G5-kysdk-ukcc": ["V1-L2-kysdk-base", "V1-L3-kysdk-system", "V2-L2-ukui-shell"],
    "G6-confiance-noyau-ia": ["V1-L10-confiance-ostree-ca", "P2-linux", "P2-openssl", "V2-L4-ia-cloud"],
    "G7-bot-ia-compositeur": ["V1-L7-kylin-bot", "V1-L8-ia-kytensor-runtime", "V1-L9-installateur-compositeur",
                              "V1-L11-approfondissement-compositeur", "V1-L12-approfondissement-kylin-bot",
                              "V1-L13-approfondissement-kytensor"],
    "G8-forks-amorcage-paquets": ["P2-grub2-amont", "P2-grub2", "P2-glibc", "P2-outils-ubuntu", "P2-reseau"],
    "G9-durcissement-sans-source": ["P2-durcissement-binaires", "P2-binaires-sans-source", "P2-dpkg"],
    "G10-boutique": ["V3-L1-boutique"],
    "G11-kylin-id": ["V3-L2-kylin-id"],
    "G12-kba-kpr-backend": ["V3-L3-kba-kpr-backend"],
    "G13-ukui-sans-surface": ["V3-L4-ukui-sans-surface"],
    "G14-ukui-reseau-peripheriques": ["V3-L6-ukui-reseau-peripheriques"],
    "G15-ukui-auth-systeme": ["V3-L7-ukui-auth-systeme"],
}


def trees(names):
    out = []
    for n in names:
        n = str(n).split(" ")[0]
        for b in ("archive", "gitee", "p2full"):
            p = f"{SRC}/{b}/{n}"
            if os.path.isdir(p) and p not in out:
                out.append(p)
    return out


docs = {}
for f in glob.glob(R + "/phase1/V*-L*.json") + glob.glob(R + "/phase2/P2-*.json"):
    d = json.load(open(f))
    docs[d["lot"]] = (f, d)

only = sys.argv[1:]
for g, lots in GROUPS.items():
    if only and g not in only:
        continue
    items = []
    for lot in lots:
        if lot not in docs:
            continue
        f, d = docs[lot]
        for i, x in enumerate(d.get("findings", [])):
            if x.get("severite") not in ("critique", "elevee"):
                continue
            items.append({"id": f"{lot}#{i}", "lot": lot, "fiche": f,
                          "arbres": trees([x.get("depot", "")] + list(d.get("composants", []))),
                          **{k: x.get(k, "") for k in ("depot", "fichier", "ligne", "type", "severite",
                                                       "registre", "extrait", "explication",
                                                       "scenario", "contre_lecture")}})
    json.dump(items, open(f"{OUT}/{g}.json", "w"), ensure_ascii=False, indent=1)
    print(g, len(items), "constats", "| sans arbre :", [x["id"] for x in items if not x["arbres"]])
