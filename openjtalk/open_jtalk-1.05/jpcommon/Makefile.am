
EXTRA_DIST = Makefile.mak

DISTCLEANFILES = *.log *.out *~

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = @CHARSET@

noinst_LIBRARIES = libjpcommon.a

libjpcommon_a_SOURCES = jpcommon.h \
                        jpcommon.c \
                        jpcommon_node.c \
                        jpcommon_label.c \
                        jpcommon_rule_euc_jp.h \
                        jpcommon_rule_shift_jis.h \
                        jpcommon_rule_utf_8.h
