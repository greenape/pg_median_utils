EXTENSION 	  = pg_median_utils# the extensions name
DATA          = $(wildcard *--*.sql)            # script files to install
TESTS         = $(wildcard test/sql/*.sql)      # use test/sql/*.sql as testfiles

# find the sql and expected directories under test
# load plpgsql into test db
# load pg_median_utils extension into test db
# dbname
REGRESS_OPTS  = --inputdir=test         \
                --load-extension=pg_median_utils \
                --load-language=plpgsql
REGRESS       = $(patsubst test/sql/%.sql,%,$(TESTS))
OBJS          = $(patsubst %.c,%.o,$(wildcard src/*.c)) # object files
# final shared library to be build from multiple source files (OBJS)
MODULE_big    = $(EXTENSION)


# postgres build stuff
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
