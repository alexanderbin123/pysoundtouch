/*
 * python interface to soundtouch (the open-source audio processing library)
 * Expose BMP detection and Shifting to python
 */

#include <soundtouch/SoundTouch.h>
#include "soundtouchmodule.h"

static PyMethodDef soundtouch_methods[] = {
    { "SoundTouch", py_soundtouch_new, METH_VARARGS, "" },
    { "BPMDetect", py_bpmdetect_new, METH_VARARGS, "" },
    { NULL, 0, 0, NULL }
};

static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT,
  "soundtouch",
  0,              /* m_doc */
  -1,             /* m_size */
  soundtouch_methods,   /* m_methods */
  NULL,           /* m_reload */
  NULL,           /* m_traverse */
  NULL,           /* m_clear */
  NULL            /* m_free */
};

PyObject *PyInit_soundtouchmodule(void) {
    PyObject *module = PyModule_Create( &moduledef );
    //PyObject *dict = PyModule_GetDict(module);

    PyModule_AddObject(module, "__version__",
			 PyUnicode_FromString(VERSION));

    if (PyErr_Occurred())
      PyErr_SetString(PyExc_ImportError, "soundtouch: init failed");

    return module;
}
