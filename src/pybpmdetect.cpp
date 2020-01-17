/*
 * python interface to soundtouch (the open-source audio processing library)
 *
 * The structure of this code was based on pymad-0.5.4
 * This is a C++ file.
 */

#include <Python.h>

extern "C" {
  #include "soundtouchmodule.h"
}
#include "pybpmdetect.h"

#if PY_VERSION_HEX < 0x01060000
#define PyObject_DEL(op) PyMem_DEL((op))
#endif

/*
PyTypeObject py_bpmdetect_t = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,
    "BPMDetect",
    sizeof(py_bpmdetect),
    0,
    // standard methods
    (destructor) py_bpmdetect_dealloc,
    (printfunc) 0,
    (getattrfunc) py_bpmdetect_getattr,
    (setattrfunc) 0,
    (cmpfunc) 0,
    (reprfunc) 0,
    // type categories
    0, // as number
    0, // as sequence
    0, // as mapping
    0, // hash
    0, // binary
    0, // repr
    0, // getattro
    0, // setattro
    0, // as buffer
    0, // tp_flags
    NULL
};
*/

// Constructor
PyObject* py_bpmdetect_new(PyObject* self, PyObject* args) {
  py_bpmdetect* ps = NULL;
  uint sampleRate, channels;
  
  // Needs to be constructed with sampling rate and number of channels
  if (!PyArg_ParseTuple(args, "II:Soundtouch", &sampleRate, &channels)) {
    PyErr_SetString(PyExc_RuntimeError, "Requires sampling rate and number of channels (sample size must be 2)");
    return NULL;
  }

  // Create the object
  ps = PyObject_NEW(py_bpmdetect, &py_bpmdetect_t);
  ps->bpmdetect = new soundtouch::BPMDetect((int) channels, (int) sampleRate);
  ps->channels = (int) channels;

  return (PyObject*) ps;
}

// Deallocate the BPMDetect object
static void py_bpmdetect_dealloc(PyObject* self, PyObject* args) {
  py_bpmdetect* ps = PY_BPMDETECT(self);

  if (ps->bpmdetect) {
    delete ps->bpmdetect;
    ps->bpmdetect = NULL;
  }

  PyObject_DEL(self);
}

// Read in a number of samples for beat detection
static PyObject* py_bpmdetect_put_samples(PyObject* self, PyObject* args) {
  py_bpmdetect* ps = PY_BPMDETECT(self);
  int buflen;
  char* transfer;

  // Needs to be called with a string of samples
  if (!PyArg_ParseTuple(args, "s#", &transfer, &buflen)) {
    PyErr_SetString(PyExc_TypeError, "invalid argument");
	return NULL;
  }

  // Move all into our char-short union
  for (int ii = 0; ii < buflen; ii++)
    ps->buffer.chars[ii] = transfer[ii];

  // Input them into the BMP detector
  ps->bpmdetect->inputSamples(ps->buffer.shorts, (uint) buflen / (2 * ps->channels));

  Py_INCREF(Py_None);
  return Py_None;
}

// Perform the beat detection algorithm
// return the beats per minute
static PyObject* py_bpmdetect_get_bpm(PyObject* self, PyObject* args) {
  py_bpmdetect* ps = PY_BPMDETECT(self);

  // Return the BPM of the input samples
  float bpm = ps->bpmdetect->getBpm();
  
  return PyFloat_FromDouble(bpm);
}

/* housekeeping */

static PyMethodDef bpmdetect_methods[] = {
    { "put_samples", py_bpmdetect_put_samples, METH_VARARGS, "" },
    { "get_bpm", py_bpmdetect_get_bpm, METH_VARARGS, "" },
    { NULL, 0, 0, NULL }
};

PyTypeObject py_bpmdetect_t = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "BPMDetect",                   /* tp_name */
    sizeof(py_bpmdetect),          /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor) py_bpmdetect_dealloc,      /* tp_dealloc */
    0,                              /* tp_vectorcall_offset */
    (getattrfunc) py_bpmdetect_getattr,     /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_as_async */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,             /* tp_flags */
    "Soundtouch",                   /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
    bpmdetect_methods,              /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)py_bpmdetect_new,    /* tp_init */
    0,                              /* tp_alloc */
    PyType_GenericNew,              /* tp_new */
};

// Extract information from the bpmdetect object
static PyObject* py_bpmdetect_getattr(PyObject* self, char* name) {
  //return Py_FindMethod(bpmdetect_methods, self, name);
  return PyObject_GenericGetAttr((PyObject *)self, PyUnicode_FromString(name));
}
