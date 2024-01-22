MODULE_big = funnel_analysis
MODULES = funnel_analysis
OBJS = funnel_analysis.o

EXTENSION = funnel_analysis
DATA = funnel_analysis--*.sql
DOCS = README.md

#only for mac os
#funnel_analysis.o: override CFLAGS += -bundle -flat_namespace -undefined suppress -I/usr/include

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)