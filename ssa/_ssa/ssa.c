#include <Python.h>
#include <numpy/arrayobject.h>
#include <stdio.h>
#include <gsl_randist.h>
#include <gsl_rng.h>
#include <sys/time.h>
#include <complex.h>
#include "two_state.c"
#include "telegraph.c"
#include "telegraph_constant.c"
#include "telegraph_hill.c"

static PyMethodDef SSAMethods[] = {
    {"two_state", two_state, METH_VARARGS, "SSA in C"},
    //{"telegraph", telegraph, METH_VARARGS, "SSA in C"},
    {"telegraph_constant", telegraph_constant, METH_VARARGS, "SSA in C"},
    {"telegraph_hill", telegraph_hill, METH_VARARGS, "SSA in C"},
    {NULL, NULL, 0, NULL},
};


static struct PyModuleDef _ssa = {
    PyModuleDef_HEAD_INIT,
    "_ssa",
    "Python interface for IF network in C",
    -1,
    SSAMethods
};

PyMODINIT_FUNC PyInit__ssa(void) {
    import_array();
    return PyModule_Create(&_ssa);
}
