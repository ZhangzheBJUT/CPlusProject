#!/bin/bash

##only for mac
cd ..
##cc -v -c funnel_analysis.c -I /usr/local/Cellar/postgresql/11.5/include/server
cc -v -c funnel_analysis.c -I /usr/local/Cellar/postgresql/12.3_4/include/postgresql/server
cc -v -bundle -flat_namespace -undefined suppress -o funnel_analysis.so funnel_analysis.o