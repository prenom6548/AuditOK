# Tracking [gpg.fail](https://gpg.fail/) issue status

## 1 https://gpg.fail/detached
Multiple Plaintext Attack on Detached PGP Signatures in GnuPG
* Upstream issue: https://dev.gnupg.org/T7903
* Upstream response: unknown status 2.2 / 2.4.8+git / master
* CVE: N/A
* Bug-Debian:
* Debian-Status: fixed in 2.4.8-5 and 2.4.7-21+deb13u1 and 2.2.40-1.1+deb12u2

## 2 https://gpg.fail/filename
  GnuPG Accepts Path Separators and Path Traversals in Literal Data "Filename" Field
* Upstream issue: https://dev.gnupg.org/T7908
* Upstream response: unfixed 2.2 / unfixed 2.4 / master
* CVE:
* Bug-Debian:
* Debian-Status: fixed in 2.4.8-6 and 2.4.7-21+deb13u1 and 2.2.40-1.1+deb12u2

## 3 https://gpg.fail/formfeed
  Cleartext Signature Plaintext Truncated for Hash Calculation
* Upstream issue:
* Upstream response:
* CVE: CVE-2025-68972
* Bug-Debian: https://bugs.debian.org/1124220
* Debian-Status:

## 4 https://gpg.fail/malleability
  Encrypted message malleability checks are incorrectly enforced causing plaintext recovery attacks
* Upstream issue:
* Upstream response:
* CVE:
* Bug-Debian:
* Debian-Status:

## 5 https://gpg.fail/memcpy
  Memory Corruption in ASCII-Armor Parsing
* Upstream issue: https://dev.gnupg.org/T7906
* Upstream response: fixed 2.2.51 / fixed 2.4.8+git / master
* CVE: CVE-2025-68973
* Debian-Bug: https://bugs.debian.org/1124221
* Debian-Status: fixed in 2.4.8-5 and 2.4.7-21+deb13u1 and 2.2.40-1.1+deb12u2

## 6 https://gpg.fail/minisign
  Trusted comment injection (minisign)
* not relevant for gnupg - minisign -issue

## 7 https://gpg.fail/notdash
  Cleartext Signature Forgery in the NotDashEscaped header implementation in GnuPG
* Upstream issue:
* Upstream response:
* CVE:
* Bug-Debian:
* Debian-Status:

## 8 https://gpg.fail/notsoclear
  OpenPGP Cleartext Signature Framework Susceptible to Format Confusion
* Upstream issue:
* Upstream response:
* CVE:
* Bug-Debian:
* Debian-Status:

## 9 https://gpg.fail/noverify
  GnuPG Output Fails To Distinguish Signature Verification Success From Message Content
* Upstream issue:
* Upstream response:
* CVE:
* Bug-Debian:
* Debian-Status:

## 10 https://gpg.fail/nullbyte
  Cleartext Signature Forgery in GnuPG
* Upstream issue: https://dev.gnupg.org/T7902
* Upstream response: open
* CVE:
* Bug-Debian:
* Debian-Status:

## 11 https://gpg.fail/polyglot
  Radix64 Line-Truncation Enabling Polyglot Attacks
* Upstream issue: https://dev.gnupg.org/T7905
* Upstream response: notabug,willnotfix,patchavailable
* CVE:
* Bug-Debian:
* Debian-Status:

## 12 https://gpg.fail/sha1
  GnuPG may downgrade digest algorithm to SHA1 during key signature checking
* Upstream issue: https://dev.gnupg.org/T7904
* Upstream response: fixed 2.2.51 / 2.4.8+git / master
* CVE:
* Bug-Debian:
* Debian-Status: fixed in 2.4.8-5 and 2.4.7-21+deb13u1 and 2.2.40-1.1+deb12u2

## 13 https://gpg.fail/trust
  GnuPG Trust Packet Parsing Enables Adding Arbitrary Subkeys
* Upstream issue:
* Upstream response:
* CVE:
* Bug-Debian:
* Debian-Status:

## 14 https://gpg.fail/trustcomment
  Trusted comment Injection (minisign)
* not relevant for gnupg - minisign -issue
