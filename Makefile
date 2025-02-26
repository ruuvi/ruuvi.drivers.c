CXX=gcc

# csv, errorfile, fullhtml, html, tasklist, xml
LOG_FORMAT=fullhtml
DOXYGEN_DIR=./doxygen

SDK_ROOT := nRF5_SDK_15.3.0_59ac345
PROJ_DIR := .
include ${PROJ_DIR}/gcc_sources.make

CFLAGS  = -c -Wall -pedantic -Wno-variadic-macros -Wno-long-long -Wno-shadow -std=c11
CFLAGS += -DRUUVI_RUN_TESTS
CFLAGS += -DBME280_FLOAT_ENABLE
OFLAGS=-g3
LDFLAGS=
DFLAGS=
INCLUDES=${COMMON_INCLUDES}
INCLUDES+=nRF5_SDK_15.3.0_59ac345/components/softdevice/s132/headers
INC_PARAMS=$(foreach d, $(INCLUDES), -I$d)
SOURCES=${RUUVI_PRJ_SOURCES} ${RUUVI_LIB_SOURCES}
ANALYSIS=$(SOURCES:.c=.a)
OBJECTS=$(SOURCES:.c=.o)
LIBRARY=build/ruuvi.drivers.a
SONAR=driver_analysis

# Tag on this commit
TAG := $(shell git describe --tags --exact-match)
# Commit hash from git
COMMIT := $(shell git rev-parse --short HEAD)
VERSION := $(if $(TAG),$(TAG),$(COMMIT))

.PHONY: clean sync doxygen astyle sonar all

all: clean sync astyle doxygen sonar $(SOURCES) $(LIBRARY)

.c.o:
# Build
	$(CXX) $(CFLAGS) $< $(DFLAGS) $(INC_PARAMS) $(OFLAGS) -o $@

doxygen:
	export PROJECT_VERSION=$(VERSION) 
	doxygen


$(LIBRARY): $(OBJECTS)
	ar rcs $@ $^

sonar: $(SOURCES) $(SONAR)
$(SONAR): $(ANALYSIS)

.c.a:
# Build
	$(CXX) $(CFLAGS) $< $(DFLAGS) $(INC_PARAMS) $(OFLAGS) -o $@

astyle:
	astyle --project=".astylerc" --recursive \
			  --exclude="src/nrf5_sdk15_platform/ruuvi.nrf_sdk15_3_overrides.c" \
			  "src/*.h" \
			  "src/*.c"
	astyle --project=".astylerc" --recursive \
			  "test/*.c"

clean:
	rm -f $(OBJECTS) $(LIBRARY) $(ANALYSIS) $(SONAR)
	rm -rf $(DOXYGEN_DIR)/html
	rm -rf $(DOXYGEN_DIR)/latex
	rm -f *.gcov

sync:
	@echo Synchronizing GIT...
	# TODO: reject if repo is not clean
	git submodule update --init --recursive
	git submodule sync --recursive
	git submodule update --init --recursive

test_all:
	rm -rf build
	CEEDLING_MAIN_PROJECT_FILE=./project.yml ceedling test:all
	CEEDLING_MAIN_PROJECT_FILE=./project.yml ceedling gcov:all utils:gcov
	CEEDLING_MAIN_PROJECT_FILE=./project_ext_adv_48.yml ceedling test:all
	CEEDLING_MAIN_PROJECT_FILE=./project_ext_adv_48.yml ceedling gcov:all utils:gcov
	gcov  -b -c build/gcov/out/*.gcno

