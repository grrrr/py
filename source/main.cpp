/* 

py - python script object for PD and MaxMSP

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

-------------------------------------------------------------------------

*/

#include <flext.h>
#include <Python.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 201)
#error You need at least flext version 0.2.1 
#endif

#define PY__VERSION "0.0.1"


#define I int
#define C char
#define V void
#define BL bool
#define F float
#define D double


class py:
	public flext_base
{
	FLEXT_HEADER_S(py,flext_base,setup)

public:
	py(I argc,t_atom *argv);
	~py();

protected:
	V m_method_(I n,const t_symbol *s,I argc,t_atom *argv);

	V work(const t_symbol *s,I argc,t_atom *argv); 

	virtual V m_bang() { work(sym_bang,0,NULL); }
	virtual V m_reset(I argc,t_atom *argv);
	virtual V m_set(I argc,t_atom *argv);

	virtual V m_help();

	// methods for python arguments
	virtual V m_py_list(I argc,t_atom *argv) { work(sym_list,argc,argv); }
	virtual V m_py_float(I argc,t_atom *argv) { work(sym_float,argc,argv); }
	virtual V m_py_int(I argc,t_atom *argv) { work(sym_int,argc,argv); }
	virtual V m_py_any(const t_symbol *s,I argc,t_atom *argv) { work(s,argc,argv); }

	C *sName,*sFunc;
	I hName;


	class lookup {
	public:
		lookup(I hash,PyObject *mod);
		~lookup();

		V Set(PyObject *mod);
		V Add(lookup *l);

		I modhash;
		PyObject *module,*dict;
		lookup *nxt;
	};

	static lookup *modules;
	static I pyref;


	V SetArgs(I argc,t_atom *argv);
	V ImportModule(const C *name);
	V SetModule(I hname,PyObject *module);
	V ResetModule();
	PyObject *GetModule();
	V SetFunction(const C *name);
	PyObject *GetFunction();

	static C *strdup(const C *s);

private:
	enum retval { nothing,atom,tuple,list };

	static V setup(t_class *);
	
	FLEXT_CALLBACK(m_bang)
	FLEXT_CALLBACK_G(m_reset)
	FLEXT_CALLBACK_G(m_set)

	FLEXT_CALLBACK_G(m_py_float)
	FLEXT_CALLBACK_G(m_py_list)
	FLEXT_CALLBACK_G(m_py_int)
	FLEXT_CALLBACK_A(m_py_any)
};


I py::pyref = 0;
py::lookup *py::modules = NULL;

py::lookup::lookup(I hname,PyObject *mod):
	modhash(hname),
	module(NULL),dict(NULL),
	nxt(NULL)
{
	Set(mod);
}

py::lookup::~lookup()
{
	if(module) Py_DECREF(module);
	if(nxt) delete nxt;
}

V py::lookup::Set(PyObject *mod)
{
    if(module) Py_DECREF(module);
	dict = PyModule_GetDict(module = mod);
}

V py::lookup::Add(lookup *l)
{
	if(nxt) nxt->Add(l);
	else nxt = l;
}


V py::setup(t_class *) 
{
	post("py %s - python script object, (C)2002 Thomas Grill",PY__VERSION);
	post("");

	py::pyref = 0;
	py::modules = NULL;
}

// make implementation of a tilde object with one float arg
FLEXT_NEW_G("py",py)


C *py::strdup(const C *s) 
{
	if(!s) return NULL;
	I len = strlen(s);
	C *ret = new C[len+1];
	strcpy(ret,s);
	return ret;
}


V py::SetArgs(I argc,t_atom *argv)
{
	// script arguments
	I i;
	C **sargv = new C *[argc];
	for(i = 0; i < argc; ++i) {
		sargv[i] = new C[256];
		GetAString(argv[i],sargv[i],255);
	}

	// the arguments to the module are only recognized once! (at first use in a patcher)
	PySys_SetArgv(argc,sargv);

	for(i = 0; i < argc; ++i) delete[] sargv[i];
	delete[] sargv;
}

V py::ImportModule(const C *name)
{
	if(!name) return;

	if(sName) delete[] sName;
	sName = strdup(name);

	PyObject *pName = PyString_FromString(sName);
	hName = PyObject_Hash(pName);

	PyObject *pModule = PyImport_Import(pName);
	if (!pModule) {
//		post("%s: python script %s not found or init error",thisName(),sName);
		PyErr_Print();
	}
	else {
		SetModule(hName,pModule);
	}

	Py_DECREF(pName);
}

V py::SetModule(I hname,PyObject *module)
{
	lookup *l;
	for(l = modules; l && l->modhash != hname; l = l->nxt);

	if(l) 
		l->Set(module);
	else {
		lookup *mod = new lookup(hname,module); 
		if(modules) modules->Add(mod);
		else modules = mod;
	}
}

V py::ResetModule()
{
	lookup *l;
	for(l = modules; l && l->modhash != hName; l = l->nxt);
	if(l && l->module) {
		PyObject *newmod = PyImport_ReloadModule(l->module);
		if(!newmod) 
			PyErr_Print();
		else {
			l->Set(newmod);
		}
	}
}

PyObject *py::GetModule()
{
	lookup *l;
	for(l = modules; l && l->modhash != hName; l = l->nxt);
	return l?l->module:NULL;
}

V py::SetFunction(const C *name)
{
	if(sFunc) delete[] sFunc;
	sFunc = strdup(name);
}

PyObject *py::GetFunction()
{
	lookup *l;
	for(l = modules; l && l->modhash != hName; l = l->nxt);
	return l?PyDict_GetItemString(l->dict,sFunc):NULL;
}


py::py(I argc,t_atom *argv):
	sName(NULL),sFunc(NULL)
{ 
	AddInAnything(2);  
	AddOutAnything();  
	SetupInOut();  // set up inlets and outlets

	FLEXT_ADDBANG(0,m_bang);
	FLEXT_ADDMETHOD_(0,"reset",m_reset);
	FLEXT_ADDMETHOD_(0,"set",m_set);

	FLEXT_ADDMETHOD_(1,"float",m_py_float);
	FLEXT_ADDMETHOD_(1,"int",m_py_int);
	FLEXT_ADDMETHOD(1,m_py_list);
	FLEXT_ADDMETHOD(1,m_py_any);


    if(!(pyref++)) Py_Initialize();


	C dir[1024];
#ifdef PD
	// uarghh... pd doesn't show it's path for extra modules

	C *name;
	I fd = open_via_path("",GetString(argv[0]),".py",dir,&name,sizeof(dir),0);
	if(fd > 0) close(fd);
	else name = NULL;
#elif defined(MAXMSP)
	*dir = 0;
#endif

	// set script path
	PySys_SetPath(dir);

	if(argc > 2) SetArgs(argc-2,argv+2);

	// init script module
	if(!IsString(argv[0])) 
		post("%s - script name argument is invalid");
	else
		ImportModule(GetString(argv[0]));

	// set function name
	if(!IsString(argv[1])) 
		post("%s - function name argument is invalid");
	else
		SetFunction(GetString(argv[1]));
}

py::~py()
{
	// pDict and pFunc are borrowed and must not be Py_DECREF-ed 

//    if(pModule) Py_DECREF(pModule);
//    if(pName) Py_DECREF(pName);

    if(!(--pyref)) {
		delete modules; modules = NULL;
		Py_Finalize();
	}
}

V py::m_method_(I n,const t_symbol *s,I argc,t_atom *argv)
{
	post("%s - no method for type %s",thisName(),GetString(s));
}

V py::m_reset(I argc,t_atom *argv)
{
	if(argc > 2) SetArgs(argc,argv);

	ResetModule();
}

V py::m_set(I argc,t_atom *argv)
{
	I ix = 0;
	if(argc >= 2) {
		if(!IsString(argv[ix])) {
			post("%s - script name is not valid",thisName());
			return;
		}
		const C *sn = GetString(argv[ix]);
		if(strcmp(sn,sName)) ImportModule(sn);
		++ix;
	}

	if(!IsString(argv[ix])) 
		post("%s - function name is not valid",thisName());
	else
		SetFunction(GetString(argv[ix]));

}

V py::m_help()
{
	post("py %s - python script object, (C)2002 Thomas Grill",PY__VERSION);
#ifdef _DEBUG
	post("compiled on " __DATE__ " " __TIME__);
#endif

	post("Arguments: %s [script name] [function name]",thisName());

	post("Inlet 1:messages to control the py object");
	post("      2:call python function with message as argument(s)");
	post("Outlet: 1:return values from python function");	
	post("Methods:");
	post("\thelp: shows this help");
	post("\tbang: call script without arguments");
	post("\tset [script name] [function name]: set (script and) function name");
	post("\treset: reload python script");
	post("");
}


V py::work(const t_symbol *s,I argc,t_atom *argv)
{
    PyObject *pArgs, *pValue;

	PyObject *pFunc = GetFunction();

	if(pFunc && PyCallable_Check(pFunc)) {
		BL any = s && s != sym_bang && s != sym_float && s != sym_int && s != sym_symbol && s != sym_list && s != sym_pointer;

		pArgs = PyTuple_New(any?argc+1:argc);

		I ix = 0;

		if(any) {
			pValue = PyString_FromString(GetString(s));

			if(!pValue) 
				post("%s: cannot convert method header",thisName());

			/* pValue reference stolen here: */
			PyTuple_SetItem(pArgs, ix++, pValue); 
		}

		for(I i = 0; i < argc; ++i) {
			pValue = NULL;
			
			if(IsFloat(argv[i])) pValue = PyFloat_FromDouble((D)GetFloat(argv[i]));
			else if(IsInt(argv[i])) pValue = PyInt_FromLong(GetInt(argv[i]));
			else if(IsSymbol(argv[i])) pValue = PyString_FromString(GetString(argv[i]));
			else if(IsPointer(argv[i])) pValue = NULL; // not handled

			if(!pValue) {
				post("%s: cannot convert argument %i",thisName(),any?i+1:i);
				continue;
			}

			/* pValue reference stolen here: */
			PyTuple_SetItem(pArgs, ix++, pValue); 
		}


		pValue = PyObject_CallObject(pFunc, pArgs);
		if (pValue != NULL) {
			// analyze return value or tuple

			I rargc = 0;
			BL ok = true;
			retval tp = nothing;

			if(!PyObject_Not(pValue)) {
				if(PyTuple_Check(pValue)) {
					rargc = PyTuple_Size(pValue);
					tp = tuple;
				}
				else if(PyList_Check(pValue)) {
					rargc = PyList_Size(pValue);
					tp = list;
				}
				else {
					rargc = 1;
					tp = atom;
				}
			}

			t_atom *ret = new t_atom[rargc];

			for(I ix = 0; ix < rargc; ++ix) {
				PyObject *arg;
				switch(tp) {
					case tuple: arg = PyTuple_GetItem(pValue,ix); break;
					case list: arg = PyList_GetItem(pValue,ix); break;
					default: arg = pValue;
				}

				if(PyInt_CheckExact(arg)) SetFlint(ret[ix],PyInt_AsLong(arg));
				else if(PyFloat_Check(arg)) SetFloat(ret[ix],(F)PyFloat_AsDouble(arg));
				else if(PyString_Check(arg)) SetString(ret[ix],PyString_AsString(arg));
				else {
					post("%s: Could not convert return argument",thisName());
					ok = false;
				}
				// No DECREF for arg -> borrowed from pValue!
			}

			Py_DECREF(pValue);

			if(rargc && ok) ToOutList(0,rargc,ret);
			delete[] ret;
		}
		else {
			PyErr_Print();
//			post("%s: python function call failed",thisName());
		}
		if(pArgs) Py_DECREF(pArgs);
	}
	else {
		post("%s: no function defined",thisName());
	}
}
