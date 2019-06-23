#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <token.h>

#if PY_MAJOR_VERSION == 3
#  if PY_MINOR_VERSION == 5
#    include "v35tokenizer.h"
#  elif PY_MINOR_VERSION == 6
#    include "v36tokenizer.h"
#  elif PY_MINOR_VERSION == 7
#    include "v37tokenizer.h"
#  elif PY_MINOR_VERSION >= 8
#    include "v38tokenizer.h"
#  else
#    error "Only Python 3.5 and higher are supported"
#  endif
#else
#  error "Python 2 is not supported"
#endif

typedef struct {
    PyObject_HEAD
    struct tok_state *tok;
} CTokObject;

static void
CTok_dealloc(CTokObject *self)
{
    if (self->tok != NULL) {
        PyTokenizer_Free(self->tok);
        self->tok = NULL;
    }

    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
CTok_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    CTokObject *self = (CTokObject *) type->tp_alloc(type, 0);
    if (self == NULL)
        return NULL;

    self->tok = NULL;
    return (PyObject *) self;
}

static int
CTok_init(CTokObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"input", NULL};
    PyObject *input;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "S", kwlist, &input))
        return -1;

    char *bytes = PyBytes_AsString(input);
    if (bytes == NULL)
        return -1;

    self->tok = PyTokenizer_FromString(bytes, 0);
    if (self->tok == NULL)
        return -1;

    return 0;
}

static PyObject *
CTok_get_raw(CTokObject *self, PyObject *Py_UNUSED(ignored))
{
    if (self->tok == NULL) {
        PyErr_SetString(PyExc_ValueError, "Uninitalized tokenizer");
        return NULL;
    }

    char *start = NULL, *end = NULL;
    int type = PyTokenizer_Get(self->tok, &start, &end);
    int istart = -1, iend = -1;
    if (start != NULL)
        istart = start - self->tok->input;
    if (end != NULL)
        iend = end - self->tok->input;
    return Py_BuildValue("(iii)", type, istart, iend);
}

static PyObject *
CTok_get(CTokObject *self, PyObject *Py_UNUSED(ignored))
{
    if (self->tok == NULL) {
        PyErr_SetString(PyExc_ValueError, "Uninitalized tokenizer");
        return NULL;
    }

    char *start = NULL, *end = NULL;
    int type = PyTokenizer_Get(self->tok, &start, &end);
    if (type == ERRORTOKEN) {
        PyErr_Format(PyExc_SyntaxError, "error at line %d", self->tok->lineno);
        return NULL;
    }
    if (type == ENDMARKER) {
        PyErr_Format(PyExc_StopIteration, "end of input at line %d", self->tok->lineno);
        return NULL;
    }

    PyObject *value = NULL;
    if (start == NULL || end == NULL) {
        value = Py_None;
        Py_INCREF(value);
    }
    else {
        value = PyBytes_FromStringAndSize(start, end-start);
        if (value == NULL)
            return NULL;
    }

    // After parsetok.c
    struct tok_state *tok = self->tok;
#if PY_MINOR_VERSION >= 8
    int lineno = type == STRING ? tok->first_lineno : tok->lineno;
    const char *line_start = type == STRING ? tok->multi_line_start : tok->line_start;
#else
    int lineno = tok->lineno;
    const char *line_start = tok->line_start;
#endif
    int end_lineno = tok->lineno;
    int col_offset = -1, end_col_offset = -1;
    if (start != NULL && start >= line_start)
        col_offset = start - line_start;
    if (end != NULL && end >= tok->line_start)
        end_col_offset = end - tok->line_start;

    return Py_BuildValue("(iO(ii)(ii))", type, value, lineno, col_offset, end_lineno, end_col_offset);
}

static PyObject *
CTok_input(CTokObject *self)
{
    if (self->tok == NULL) {
        PyErr_SetString(PyExc_ValueError, "Uninitalized tokenizer");
        return NULL;
    }

    if (self->tok->input == NULL) {
        Py_RETURN_NONE;
    }

    return PyBytes_FromString(self->tok->input);
}

static PyObject *
CTok_iter(PyObject *self)
{
    Py_INCREF(self);
    return self;
}

static PyObject *
CTok_iternext(PyObject *self)
{
    return CTok_get((CTokObject *)self, NULL);
}

static PyMethodDef CTok_methods[] = {
    {"get", (PyCFunction) CTok_get, METH_NOARGS,
     "Get the next token\n"
     "\n"
     "Returns (type, string, (line, col), (endline, endcol))."
    },
    {"get_raw", (PyCFunction) CTok_get_raw, METH_NOARGS,
     "Get the next token without allocating much\n"
     "\n"
     "Returns (type, start, end) where start and end point into self.input()."
    },
    {"input", (PyCFunction) CTok_input, METH_NOARGS,
     "Returns the input string as seen by the tokenizer\n"
     "\n"
     "This may differ from the input you passed in "
     "due to encoding and newline normalization."
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject CTokType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ctok.CTok",
    .tp_doc = "C Tokenizer",
    .tp_basicsize = sizeof(CTokObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = CTok_new,
    .tp_init = (initproc) CTok_init,
    .tp_dealloc = (destructor) CTok_dealloc,
    .tp_methods = CTok_methods,
    .tp_iter = CTok_iter,
    .tp_iternext = CTok_iternext,
};

static struct PyModuleDef ctokmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "ctok",
    .m_doc = "Expose CPython's tokenizer as a Python class",
};

PyMODINIT_FUNC
PyInit_ctok(void)
{
    if (PyType_Ready(&CTokType) < 0)
        return NULL;

    PyObject *m = PyModule_Create(&ctokmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&CTokType);
    PyModule_AddObject(m, "CTok", (PyObject *) &CTokType);

    return m;
}
