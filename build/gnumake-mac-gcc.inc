DEFS += -DPY_EXPORTS
LIBS +=  -F/Library/Frameworks -framework Python

ifdef PY_NUMARRAY
DEFS += -DPY_NUMARRAY
endif
ifdef PY_NUMPY
DEFS += -DPY_NUMPY
INCPATH += -I/Library/Frameworks/Python.framework/Versions/2.5/lib/python2.5/site-packages/numpy/core/include
#INCPATH +=  -F/System/Library/Frameworks -I/Library/Python/2.5/site-packages/numpy/core/include
endif
ifdef PY_NUMERIC
DEFS += -DPY_NUMERIC
endif

ifdef PY_USE_GIL
DEFS += -DPY_USE_GIL
endif

ifdef PY_USE_INOFFICIAL
DEFS += -DPY_USE_INOFFICIAL
endif
