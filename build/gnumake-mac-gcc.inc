DEFS += -DPY_EXPORTS

ifdef PY_NUMPY
DEFS += -DPY_NUMPY
endif

ifdef PY_NUMARRAY
DEFS += -DPY_NUMARRAY
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

ifeq ($(PY_KIND),conda)

ifndef PY_CONDA_ROOT
$(error PY_CONDA_ROOT is undefined)
endif

DEFS += -DPY_INTERPRETER=$(PY_CONDA_ROOT)/bin/python
LIBS += $(PY_CONDA_ROOT)/lib/libpython$(PY_MAJOR_VERSION).$(PY_MINOR_VERSION).dylib
INCPATH += -I$(PY_CONDA_ROOT)/include
INCPATH += -I$(PY_CONDA_ROOT)/include/python$(PY_MAJOR_VERSION).$(PY_MINOR_VERSION)
LDFLAGS += -rpath $(PY_CONDA_ROOT)/lib

ifdef PY_NUMPY
INCPATH += -I$(PY_CONDA_ROOT)/lib/python$(PY_MAJOR_VERSION).$(PY_MINOR_VERSION)/site-packages/numpy/core/include
endif

else

DEFS += -DPY_USE_FRAMEWORK

# don't use -framework Python, since this will stick to the default system version

_LOCAL_FRAMEWORK := /Library/Frameworks/Python.framework/Versions/$(PY_MAJOR_VERSION).$(PY_MINOR_VERSION)
_SYSTEM_FRAMEWORK := /System/Library/Frameworks/Python.framework/Versions/$(PY_MAJOR_VERSION).$(PY_MINOR_VERSION)
_LOCAL_LIBRARY := /Library/Python/$(PY_MAJOR_VERSION).$(PY_MINOR_VERSION)
_SYSTEM_LIBRARY := /System/Library/Python/$(PY_MAJOR_VERSION).$(PY_MINOR_VERSION)

INCPATH += -F/Library/Frameworks -framework Python

ifeq ($(PY_KIND),system)
LIBS += $(_SYSTEM_FRAMEWORK)/Python
INCPATH += -I$(_SYSTEM_FRAMEWORK)/Headers
else
LIBS += $(_LOCAL_FRAMEWORK)/Python
INCPATH += -I$(_LOCAL_FRAMEWORK)/Headers
endif

ifdef PY_NUMPY
INCPATH += -I$(_LOCAL_LIBRARY)/python$(PY_MAJOR_VERSION).$(PY_MINOR_VERSION)/site-packages/numpy/core/include

ifeq ($(PY_KIND),system)
INCPATH += -I$(_SYSTEM_FRAMEWORK)/lib/python$(PY_MAJOR_VERSION).$(PY_MINOR_VERSION)/site-packages/numpy/core/include
INCPATH += -I$(_SYSTEM_FRAMEWORK)/Extras/lib/python/numpy/core/include
else
INCPATH += -I$(_LOCAL_FRAMEWORK)/lib/python$(PY_MAJOR_VERSION).$(PY_MINOR_VERSION)/site-packages/numpy/core/include
endif
endif

endif
