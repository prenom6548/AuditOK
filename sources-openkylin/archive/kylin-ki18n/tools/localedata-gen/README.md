# Localedata Generation for kylin-ki18n

This directory contains the minimal data generation pipeline for `kylin-ki18n`
locale data, aligned with `ki18n` data sources.

## Goals

- Use `iso-codes` JSON as canonical country/subdivision source.
- Populate country -> timezone mapping from tzdb `zone.tab`.
- Produce runtime JSON files consumed by `src/locale/data.rs`.
- Provide deterministic validation via JSON schema.

## Outputs

Generated files go to `kylin-ki18n/data/`:

- `iso_3166_1.json`
- `iso_3166_2.json`
- `timezone_coords.json`
- `MANIFEST.json`

## Quick Start

```bash
cd kylin-ki18n
python3 -m venv .venv
source .venv/bin/activate
pip install -r tools/localedata-gen/requirements.txt

make -C tools/localedata-gen localedata \
  ROOT=$(pwd) \
  ISO_CODES_DIR=/usr/share/iso-codes/json
```

Run end-to-end generation + parity tests:

```bash
make -C tools/localedata-gen localedata-check \
  ROOT=$(pwd) \
  ISO_CODES_DIR=/usr/share/iso-codes/json
```

Enable strict timezone ID parity check against ki18n (optional):

```bash
make -C tools/localedata-gen localedata-check \
  ROOT=$(pwd) \
  ISO_CODES_DIR=/usr/share/iso-codes/json \
  KI18N_TIMEZONE_TABLE=/path/to/ki18n/src/localedata/data/timezone_name_table.cpp
```

Timezone parity supports whitelist + report output:

- whitelist file: `tools/localedata-gen/config/tz-diff-whitelist.txt`
- JSON report: `data/tz-diff-report.json`

## Timezone Generation

Current implementation supports:

- `zone1970` mode (default): read `/usr/share/zoneinfo/zone1970.tab`
- optional filtering by ki18n timezone table:
  `KI18N_TIMEZONE_TABLE=/path/to/timezone_name_table.cpp`

This gives real coordinates per timezone (from tzdb), while keeping timezone ID
alignment closer to ki18n.

## Notes

- The first phase focuses on `iso-codes` parity.
- `timezone_coords.json` now comes from tzdb `zone1970.tab` representative
  coordinates.
- Later phases should replace representative points with QGIS/tz-boundary based
  polygon/nearest logic and tighten parity thresholds against C++ outputs.
