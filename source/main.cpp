/* 

py - python script object for PD and MaxMSP

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

-------------------------------------------------------------------------

*/

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 200)
#error You need at least flext version 0.2.0 
#endif


class py:
	public flext_base
{
	FLEXT_HEADER(py,flext_base)

public:
	py(I argc,t_atom *argv);
	~py();

protected:
	virtual V m_method_m(I n,const t_symbol *s,I argc,t_atom *argv); 
};

// make implementation of a tilde object with one float arg
FLEXT_GIMME("py",py)


py::py(I argc,t_atom *argv)
{ 
	add_in_anything();  
	add_out_anything();  
	setup_inout();  // set up inlets and outlets
}

py::~py()
{
}

V py::m_method_m(I n,const t_symbol *s,I argc,t_atom *argv)
{
}
