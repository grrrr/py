#ifndef __PYEXT_H
#define __PYEXT_H

/* 

py/pyext - python external object for PD and MaxMSP

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

-------------------------------------------------------------------------

*/

#include "main.h"

class pyext:
	public py
{
	FLEXT_HEADER_S(pyext,py,setup)

public:
	pyext(I argc,t_atom *argv);
	~pyext();

	static PyObject *py__init__(PyObject *self,PyObject *args);
	static PyObject *py__del__(PyObject *self,PyObject *args);
	static PyObject *py_outlet(PyObject *self,PyObject *args);
	static PyObject *py_setattr(PyObject *self,PyObject *args);
	static PyObject *py_getattr(PyObject *self,PyObject *args);

	I Inlets() const { return inlets; }
	I Outlets() const { return outlets; }

protected:
	BL m_method_(I n,const t_symbol *s,I argc,t_atom *argv);

	BL work(I n,const t_symbol *s,I argc,t_atom *argv); 

	V m_reload(I argc,t_atom *argv);
	virtual V m_help();

	const t_symbol *methname;
	PyObject *pyobj;
	I inlets,outlets;

private:
	static V setup(t_class *);

	static pyext *GetThis(PyObject *self);
	V ClearBinding();
	BL SetClssMeth(I argc,t_atom *argv);

	static PyObject *class_obj,*class_dict;
	static PyMethodDef attr_tbl[],meth_tbl[];

	// -------- bound stuff ------------------

	static t_class *px_class;

	friend class py_proxy;

	class py_proxy  // no virtual table!
	{ 
	public:
		t_object obj;			// MUST reside at memory offset 0
		PyObject *self,*func;
		t_symbol *name;

		py_proxy *nxt;

		void init(t_symbol *n,PyObject *s,PyObject *f) { name = n,self = s,func = f,nxt = NULL; }
//		bool cmp(PyObject *s,PyObject *f) const { return self == s && func == f; }
//		void init(PyObject *s,char *f) { self = s,func = f,nxt = NULL; }
//		bool cmp(PyObject *s,char *f) const { return self == s && func == f; }
		static void px_method(py_proxy *c,const t_symbol *s,int argc,t_atom *argv);
	};
	static py_proxy *px_head,*px_tail;

	static PyObject *py_bind(PyObject *self,PyObject *args);
	static PyObject *py_unbind(PyObject *self,PyObject *args);

	// ---------------------------

	PyObject *call(const C *meth,I inlet,const t_symbol *s,I argc,t_atom *argv);

	V work_wrapper(void *data); 
	BL callwork(I n,const t_symbol *s,I argc,t_atom *argv); 

	class work_data:
		public flext_base::AtomAnything
	{
	public:
		work_data(I _n,const t_symbol *_s,I _argc,t_atom *_argv): n(_n),AtomAnything(_s,_argc,_argv) {}
		I n;
	};

#ifdef FLEXT_THREADS
	FLEXT_THREAD_X(work_wrapper)
#else
	FLEXT_CALLBACK_X(work_wrapper)
#endif

	PyThreadState *pythr;

private:
	FLEXT_CALLBACK_V(m_reload)
};


#endif