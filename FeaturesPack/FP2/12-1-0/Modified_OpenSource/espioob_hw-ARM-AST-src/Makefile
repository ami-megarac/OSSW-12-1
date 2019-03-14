DEBUG := n
TARGET := espioob_hw

OBJS := ast_espioob.o 

EXTRA_CFLAGS += -I${SPXINC}/global
EXTRA_CFLAGS += -I${SPXINC}/helper
EXTRA_CFLAGS += -I${SPXINC}/espi

KBUILD_EXTRA_SYMBOLS += ${SPXINC}/espi/Module.symvers

-include ${TOOLDIR}/rules/Rules.make.drivers

