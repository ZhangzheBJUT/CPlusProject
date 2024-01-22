MODULE_big = retention_analysis
MODULES = retention_analysis
OBJS = retention_analysis.o

EXTENSION = retention_analysis
DATA = retention_analysis--*.sql
DOCS = README.md

#only for mac os
#retention_analysis.o: override CFLAGS += -bundle -flat_namespace -undefined suppress -I/usr/include

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)