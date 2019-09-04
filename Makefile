EXTENSION    = $(shell grep -m 1 '"name":' META.json | \
               sed -e 's/[[:space:]]*"name":[[:space:]]*"\([^"]*\)",/\1/')
EXTVERSION   = $(shell grep -m 1 '[[:space:]]\{8\}"version":' META.json | \
               sed -e 's/[[:space:]]*"version":[[:space:]]*"\([^"]*\)",\{0,1\}/\1/')


PG96 = $(shell $(PG_CONFIG) --version | egrep " 8\.| 9\.0| 9\.1| 9\.2| 9\.3 | 9\.4 | 9\.5" > /dev/null && echo no || echo yes)
EXTVERSION   = $(shell grep default_version $(EXTENSION).control | sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")

DOCS         = $(wildcard doc/*.md)
DATA         = $(filter-out $(wildcard sql/*--*.sql),$(wildcard sql/*.sql))
TESTS        = $(wildcard test/sql/*.sql)
REGRESS      = $(patsubst test/sql/%.sql,%,$(TESTS))
REGRESS_OPTS = --inputdir=test --load-extension=$(EXTENSION) --load-language=plpgsql
OBJS          = $(patsubst %.c,%.o,$(wildcard src/*.c)) # object files
MODULE_big    = $(EXTENSION)
PG_CONFIG    = pg_config


ifeq ($(PG96), no)
$(error Minimum version of PostgreSQL required is 9.6.0)
endif

all: sql/$(EXTENSION)--$(EXTVERSION).sql

sql/$(EXTENSION)--$(EXTVERSION).sql: sql/$(EXTENSION).sql
	cp $< $@

DATA = $(wildcard sql/*--*.sql)
EXTRA_CLEAN = sql/$(EXTENSION)--$(EXTVERSION).sql

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
