/* 

py/pyext - python external object for PD and MaxMSP

Copyright (c)2002-2005 Thomas Grill (gr@grrrr.org)
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
	pyext(int argc,const t_atom *argv);
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

	int Inlets() const { return inlets; }
	int Outlets() const { return outlets; }

protected:
	virtual bool m_method_(int n,const t_symbol *s,int argc,const t_atom *argv);

	bool work(int n,const t_symbol *s,int argc,const t_atom *argv); 

	void m_reload();
	void m_reload_(int argc,const t_atom *argv);
    void ms_args(const AtomList &a) { m_reload_(a.Count(),a.Atoms()); }
    void m_dir_() { m__dir(pyobj); }
    void mg_dir_(AtomList &lst) { GetDir(pyobj,lst); }
    void m_doc_() { m__doc(((PyInstanceObject *)pyobj)->in_class->cl_dict); }
	virtual void m_help();

	void m_get(const t_symbol *s);
	void m_set(int argc,const t_atom *argv);

	const t_symbol *methname;
	PyObject *pyobj;
	int inlets,outlets;

private:
	static void Setup(t_classid);

	static pyext *GetThis(PyObject *self);
	void SetThis();

	void ClearBinding();
	bool MakeInstance();
	bool DoInit();
	void DoExit();
    void InitInOut(int &inlets,int &outlets);

	AtomList args;

	virtual void Reload();

	static PyObject *class_obj,*class_dict;
	static PyMethodDef attr_tbl[],meth_tbl[];
	static const char *pyext_doc;

	// -------- bind stuff ------------------
	static PyObject *pyext_bind(PyObject *,PyObject *args);
	static PyObject *pyext_unbind(PyObject *,PyObject *args);

	// ---------------------------

	bool call(const char *meth,int inlet,const t_symbol *s,int argc,const t_atom *argv);

    virtual bool callpy(PyObject *fun,PyObject *args);
    static bool stcallpy(PyObject *fun,PyObject *args);

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
