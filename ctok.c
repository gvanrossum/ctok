#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <token.h>

// Bad Guido!  These are in Python/tokenizer.h which is no longer public.
extern struct tok_state *PyTokenizer_FromString(const char *, int);
extern struct tok_state *PyTokenizer_FromUTF8(const char *, int);
extern struct tok_state *PyTokenizer_FromFile(FILE *, const char*,
                                              const char *, const char *);
extern void PyTokenizer_Free(struct tok_state *);
extern int PyTokenizer_Get(struct tok_state *, char **, char **);

typedef struct {
    PyObject_HEAD
    PyObject *input;
    struct tok_state *tok;
} CTokObject;

static void
CTok_dealloc(CTokObject *self)
{
    Py_XDECREF(self->input);
    self->input = NULL;

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

    self->input = NULL;
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

    Py_INCREF(input);
    self->input = input;

    return 0;
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
        PyErr_SetString(PyExc_SyntaxError, "error token");
        return NULL;
    }
    if (type == ENDMARKER) {
        PyErr_SetString(PyExc_StopIteration, "end of input");
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
    // TODO: lineno, col_offset, end_lineno, end_col_offset.
    return Py_BuildValue("(iO)", type, value);
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
     "Get the next token; returns (type, value)"
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
