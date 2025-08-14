## Process this file with automake to produce Makefile.in

EXTRA_DIST = .cvsignore dumb-pt-html.dsssl

if DUMB_CONFIG_JADE
all-local: dumb-pt.html
else
all-local:
endif

JADE = jade
MOVE_IF_CHANGE = $(top_srcdir)/scripts/move-if-change

raw_index = $(srcdir)/../dumb-pt.index

dumb-pt.html: ../dumb-pt.sgml ../dumb-pt-index.sgml
	$(JADE) -t sgml -d $(srcdir)/dumb-pt-html.dsssl $<
	$(MOVE_IF_CHANGE) new.index $(raw_index)
