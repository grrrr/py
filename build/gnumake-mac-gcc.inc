DEFS += -DPY_EXPORTS
LIBS += -framework Python

ifdef PY_NUMARRAY
DEFS += -DPY_NUMARRAY
endif
