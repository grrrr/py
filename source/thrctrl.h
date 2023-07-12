#ifndef __THRCTRL_H
#define __THRCTRL_H

#include <stdio.h>
#include "flsupport.h"

#ifdef FLEXT_THREADS
class ThrCtrl
{
public:
    explicit ThrCtrl(flext::ThrCond * thr, bool * ctrlbool, bool init = true) 
    : _attached_boolean { ctrlbool }
    , _attached_thrcond { thr }
    {
        *_attached_boolean = init;
    }

    ~ThrCtrl()
    {
        *_attached_boolean = !*_attached_boolean;
        if(!*_attached_boolean) 
        {
            _attached_thrcond->Signal();
            fprintf(stderr, "Shutting down py/pyext thread pool.\n");
        }
    }
   
private:
    bool * _attached_boolean;
    flext::ThrCond * _attached_thrcond;
};
#endif
#endif // !__THRCTRL_H