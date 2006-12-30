DEFS += -DPY_EXPORTS
LIBS += -framework Python

ifdef PY_NUMARRAY
DEFS += -DPY_NUMARRAY
endif
ifdef PY_NUMPY
DEFS += -DPY_NUMPY
INCPATH += -I/Library/Python/2.3/site-packages/numpy/core/include
endif
ifdef PY_NUMERIC
DEFS += -DPY_NUMERIC
endif

