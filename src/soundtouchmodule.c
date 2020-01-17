/*
 * python interface to soundtouch (the open-source audio processing library)
 * Expose BMP detection and Shifting to python
 */

#include <soundtouch/SoundTouch.h>
#include "soundtouchmodule.h"

/*
static PyMethodDef soundtouch_methods[] = {
    { "soundtouch.SoundTouch", py_soundtouch_new, METH_VARARGS, "" },
    { "soundtouch.BPMDetect", py_bpmdetect_new, METH_VARARGS, "" },
    { NULL, 0, 0, NULL }
};
*/

static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT,
  "soundtouch",
  0,                    // m_doc
  -1,                   // m_size
  /*
  soundtouch_methods,   // m_methods
  NULL,                 // m_reload
  NULL,                 // m_traverse
  NULL,                 // m_clear
  NULL                  // m_free
  */
};

PyObject *PyInit_soundtouch(void) {
    extern PyTypeObject py_soundtouch_t;
    extern PyTypeObject py_bpmdetect_t;
    if ( PyType_Ready( &py_soundtouch_t ) < 0 || PyType_Ready( &py_bpmdetect_t ) < 0 )
        return NULL;

    PyObject *module = PyModule_Create( &moduledef );

    Py_INCREF( &py_soundtouch_t );
    if ( PyModule_AddObject(module, "SoundTouch", (PyObject *)&py_soundtouch_t) < 0) {
        Py_DECREF(&py_soundtouch_t);
        Py_DECREF(module);
        return NULL;
    }

    Py_INCREF( &py_bpmdetect_t );
    if ( PyModule_AddObject(module, "BPMDetect", (PyObject *)&py_bpmdetect_t) < 0) {
        Py_DECREF(&py_bpmdetect_t);
        Py_DECREF(module);
        return NULL;
    }

    PyModule_AddObject(module, "__version__",
			 PyUnicode_FromString(VERSION));

    if (PyErr_Occurred())
      PyErr_SetString(PyExc_ImportError, "soundtouch: init failed");

    return module;
}
