DEFS += -DPY_EXPORTS
LIBS += -framework Python

ifdef PY_NUMARRAY
DEFS += -DPY_NUMARRAY
endif
ifdef PY_NUMPY
DEFS += -DPY_NUMPY
endif
ifdef PY_NUMERIC
DEFS += -DPY_NUMERIC
endif

