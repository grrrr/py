/* 

py/pyext - python external object for PD and MaxMSP

Copyright (c)2002-2004 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __PYEXT_H
#define __PYEXT_H

#include "main.h"

class pyext:
	public py
{
	FLEXT_HEADER_S(pyext,py,Setup)

public:
	pyext(I argc,const t_atom *argv);
	~pyext();

	static PyObject *pyext__doc__(PyObject *,PyObject *args);
	static PyObject *pyext__init__(PyObject *,PyObject *args);
	static PyObject *pyext__del__(PyObject *,PyObject *args);

	static PyObject *pyext_outlet(PyObject *,PyObject *args);
#if FLEXT_SYS == FLEXT_SYS_PD
	static PyObject *pyext_tocanvas(PyObject *,PyObject *args);
#endif

	static PyObject *pyext_setattr(PyObject *,PyObject *args);
	static PyObject *pyext_getattr(PyObject *,PyObject *args);

	static PyObject *pyext_detach(PyObject *,PyObject *args);
	static PyObject *pyext_stop(PyObject *,PyObject *args);
	static PyObject *pyext_isthreaded(PyObject *,PyObject *);

	I Inlets() const { return inlets; }
	I Outlets() const { return outlets; }

protected:
	virtual BL m_method_(I n,const t_symbol *s,I argc,const t_atom *argv);

	BL work(I n,const t_symbol *s,I argc,const t_atom *argv); 

	V m_reload();
	V m_reload_(I argc,const t_atom *argv);
    V ms_args(const AtomList &a) { m_reload_(a.Count(),a.Atoms()); }
    V m_dir_() { m__dir(pyobj); }
    V mg_dir_(AtomList &lst) { GetDir(pyobj,lst); }
    V m_doc_() { m__doc(pyobj); }
	virtual V m_help();

	V m_get(const t_symbol *s);
	V m_set(I argc,const t_atom *argv);

	const t_symbol *methname;
	PyObject *pyobj;
	I inlets,outlets;

private:
	static V Setup(t_classid);

	static pyext *GetThis(PyObject *self);
	V ClearBinding();
	BL SetClssMeth(); //I argc,t_atom *argv);

	AtomList args;

	virtual V Reload();

	static I pyextref;
	static PyObject *class_obj,*class_dict;
	static PyMethodDef attr_tbl[],meth_tbl[];
	static const C *pyext_doc;

	// -------- bind stuff ------------------
	static PyObject *pyext_bind(PyObject *,PyObject *args);
	static PyObject *pyext_unbind(PyObject *,PyObject *args);

	// ---------------------------

	PyObject *call(const C *meth,I inlet,const t_symbol *s,I argc,const t_atom *argv);

	V work_wrapper(void *data); 
	BL callwork(I n,const t_symbol *s,I argc,const t_atom *argv); 

	class work_data:
		public flext::AtomAnything
	{
	public:
		work_data(I _n,const t_symbol *_s,I _argc,const t_atom *_argv): n(_n),AtomAnything(_s,_argc,_argv) {}
		I n;
	};

#ifdef FLEXT_THREADS
	FLEXT_THREAD_X(work_wrapper)
#else
	FLEXT_CALLBACK_X(work_wrapper)
#endif

	PyThreadState *pythr;

private:
    static bool boundmeth(flext_base *,t_symbol *sym,int argc,t_atom *argv,void *data);
    
    FLEXT_CALLBACK(m_reload)
	FLEXT_CALLBACK_V(m_reload_)
	FLEXT_CALLBACK(m_dir_)
	FLEXT_CALLGET_V(mg_dir_)
	FLEXT_CALLBACK(m_doc_)

    FLEXT_ATTRGET_V(args)
    FLEXT_CALLSET_V(ms_args)

	FLEXT_CALLBACK_S(m_get)
	FLEXT_CALLBACK_V(m_set)
};


#endif
