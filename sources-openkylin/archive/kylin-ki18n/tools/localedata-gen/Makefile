PY ?= python3
ROOT ?= $(abspath ../..)
DATA_DIR ?= $(ROOT)/data
SCHEMA_DIR ?= $(ROOT)/tools/localedata-gen/schemas
CFG ?= $(ROOT)/tools/localedata-gen/config/default.yaml
ISO_CODES_DIR ?= /usr/share/iso-codes/json
TZ_SOURCE ?=
KI18N_TIMEZONE_TABLE ?=
KI18N_TIMEZONE_COUNTRY_MAP ?=
KI18N_SPATIAL_INDEX_DATA_CPP ?=
KI18N_SPATIAL_INDEX_PROPERTIES_CPP ?=
KI18N_COUNTRY_TIMEZONE_MAP_CPP ?=
KI18N_SUBDIVISION_TIMEZONE_MAP_CPP ?=
TZ_DIFF_WHITELIST ?= $(ROOT)/tools/localedata-gen/config/tz-diff-whitelist.txt
TZ_DIFF_REPORT ?= $(ROOT)/data/tz-diff-report.json

.PHONY: all gen-iso gen-tz gen-tz-country-map gen-spatial-index gen-country-timezones validate manifest compare-tz parity-test localedata localedata-check clean

all: localedata

gen-iso:
	$(PY) $(ROOT)/tools/localedata-gen/scripts/gen_iso_json.py \
		--iso-codes-dir "$(ISO_CODES_DIR)" \
		--out-dir "$(DATA_DIR)" \
		$(if $(KI18N_TIMEZONE_TABLE),--ki18n-timezone-table "$(KI18N_TIMEZONE_TABLE)",) \
		--config "$(CFG)" \
		--verbose

gen-tz:
	$(PY) $(ROOT)/tools/localedata-gen/scripts/gen_tz_coords.py \
		--out-file "$(DATA_DIR)/timezone_coords.json" \
		--mode zone1970 \
		$(if $(TZ_SOURCE),--tz-source "$(TZ_SOURCE)",) \
		$(if $(KI18N_TIMEZONE_TABLE),--ki18n-timezone-table "$(KI18N_TIMEZONE_TABLE)",) \
		--verbose

gen-tz-country-map:
	@if [ -n "$(KI18N_TIMEZONE_COUNTRY_MAP)" ] && [ -f "$(KI18N_TIMEZONE_COUNTRY_MAP)" ]; then \
		$(PY) $(ROOT)/tools/localedata-gen/scripts/gen_timezone_country_map.py \
			--timezone-country-map-cpp "$(KI18N_TIMEZONE_COUNTRY_MAP)" \
			--out-file "$(DATA_DIR)/timezone_country_map.json" \
			--verbose; \
	else \
		echo "skip gen-tz-country-map: KI18N_TIMEZONE_COUNTRY_MAP is not set or file missing"; \
	fi

gen-spatial-index:
	@if [ -n "$(KI18N_SPATIAL_INDEX_DATA_CPP)" ] && [ -f "$(KI18N_SPATIAL_INDEX_DATA_CPP)" ] && [ -n "$(KI18N_SPATIAL_INDEX_PROPERTIES_CPP)" ] && [ -f "$(KI18N_SPATIAL_INDEX_PROPERTIES_CPP)" ]; then \
		$(PY) $(ROOT)/tools/localedata-gen/scripts/gen_spatial_index_lookup.py \
			--spatial-index-data-cpp "$(KI18N_SPATIAL_INDEX_DATA_CPP)" \
			--spatial-index-properties-cpp "$(KI18N_SPATIAL_INDEX_PROPERTIES_CPP)" \
			--out-entries-bin "$(DATA_DIR)/spatial_index_entries.bin" \
			--out-properties-json "$(DATA_DIR)/spatial_index_properties.json" \
			--verbose; \
	else \
		echo "skip gen-spatial-index: KI18N_SPATIAL_INDEX_DATA_CPP/PROPERTIES not set or file missing"; \
	fi

gen-country-timezones:
	@if [ -n "$(KI18N_COUNTRY_TIMEZONE_MAP_CPP)" ] && [ -f "$(KI18N_COUNTRY_TIMEZONE_MAP_CPP)" ] && [ -n "$(KI18N_SUBDIVISION_TIMEZONE_MAP_CPP)" ] && [ -f "$(KI18N_SUBDIVISION_TIMEZONE_MAP_CPP)" ]; then \
		$(PY) $(ROOT)/tools/localedata-gen/scripts/gen_country_timezone_maps.py \
			--country-timezone-map-cpp "$(KI18N_COUNTRY_TIMEZONE_MAP_CPP)" \
			--subdivision-timezone-map-cpp "$(KI18N_SUBDIVISION_TIMEZONE_MAP_CPP)" \
			--out-country-timezone-map-json "$(DATA_DIR)/country_timezone_map.json" \
			--out-country-timezones-from-subdiv-json "$(DATA_DIR)/country_timezones_from_subdiv.json" \
			--verbose; \
	else \
		echo "skip gen-country-timezones: KI18N_COUNTRY_TIMEZONE_MAP_CPP/..." ; \
	fi

validate:
	$(PY) $(ROOT)/tools/localedata-gen/scripts/validate_json.py \
		--schema-dir "$(SCHEMA_DIR)" \
		--data-dir "$(DATA_DIR)" \
		--verbose

manifest:
	$(PY) $(ROOT)/tools/localedata-gen/scripts/build_manifest.py \
		--data-dir "$(DATA_DIR)" \
		--out-file "$(DATA_DIR)/MANIFEST.json" \
		--config "$(CFG)" \
		--verbose

compare-tz:
	@if [ -n "$(KI18N_TIMEZONE_TABLE)" ] && [ -f "$(KI18N_TIMEZONE_TABLE)" ]; then \
		$(PY) $(ROOT)/tools/localedata-gen/scripts/compare_tz_with_ki18n.py \
			--generated "$(DATA_DIR)/timezone_coords.json" \
			--ki18n-table "$(KI18N_TIMEZONE_TABLE)" \
			--min-coverage 0.70 \
			--whitelist-file "$(TZ_DIFF_WHITELIST)" \
			--report-out "$(TZ_DIFF_REPORT)" \
			--verbose; \
	else \
		echo "skip compare-tz: KI18N_TIMEZONE_TABLE is not set or file missing"; \
	fi

localedata: gen-iso gen-tz gen-tz-country-map gen-spatial-index gen-country-timezones validate manifest compare-tz

parity-test:
	KI18N_RS_DATA_DIR="$(DATA_DIR)" cargo test --test locale_parity_test

localedata-check: localedata parity-test

clean:
	rm -f "$(DATA_DIR)/iso_3166_1.json" \
		"$(DATA_DIR)/iso_3166_2.json" \
		"$(DATA_DIR)/timezone_coords.json" \
		"$(DATA_DIR)/timezone_country_map.json" \
		"$(DATA_DIR)/spatial_index_entries.bin" \
		"$(DATA_DIR)/spatial_index_properties.json" \
		"$(DATA_DIR)/country_timezone_map.json" \
		"$(DATA_DIR)/country_timezones_from_subdiv.json" \
		"$(DATA_DIR)/MANIFEST.json"
