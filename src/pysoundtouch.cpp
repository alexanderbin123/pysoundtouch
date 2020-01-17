/*
 * python interface to soundtouch (the open-source audio processing library)
 *
 * The structure of this code was based on pymad-0.5.4
 * This is a C++ file.
 */

#include <Python.h>
#include <vector>

extern "C" {
  #include "soundtouchmodule.h"
}
#include "pysoundtouch.h"

#if PY_VERSION_HEX < 0x01060000
#define PyObject_DEL(op) PyMem_DEL((op))
#endif

/* For debugging:
#include "WavFile.h"
WavOutFile* inputs = new WavOutFile("/Users/jrising/inputs.wav", 44100, 16, 1);
WavOutFile* outputs = new WavOutFile("/Users/jrising/outputs.wav", 44100, 16, 1);*/

/*
PyTypeObject py_soundtouch_t = {
    PyObject_HEAD_INIT(&PyType_Type)
    .tp_name = "Soundtouch",
    .tp_basicsize = sizeof(py_soundtouch),
    // standard methods
    .tp_dealloc = (destructor) py_soundtouch_dealloc,
    .tp_getattr  = (getattrfunc) py_soundtouch_getattr
};
*/

// Constructor
int py_soundtouch_init(PyObject* self, PyObject* args, PyObject *kwds) {
  py_soundtouch* ps = NULL;
  uint sampleRate, channels;
  
  // Needs to be given the sampling rate and number of channels
  if (!PyArg_ParseTuple(args, "II:Soundtouch", &sampleRate, &channels)) {
    PyErr_SetString(PyExc_RuntimeError, "Requires sampling rate and number of channels (sample size must be 2)");
    return 0;
  }

  // Create the object
  ps = PY_SOUNDTOUCH(self);
  //ps = PyObject_NEW(py_soundtouch, &py_soundtouch_t);
  ps->soundtouch = new soundtouch::SoundTouch();
  ps->channels = (int) channels;
  ps->soundtouch->setSampleRate(sampleRate);
  ps->soundtouch->setChannels(channels);

  return 1;
}

// Deallocate the SoundTouch object
static void py_soundtouch_dealloc(PyObject* self, PyObject* args) {
  py_soundtouch* ps = PY_SOUNDTOUCH(self);

  if (ps->soundtouch) {
    delete ps->soundtouch;
    ps->soundtouch = NULL;
  }

  PyObject_DEL(self);
}

// Set the rate of playback
static PyObject* py_soundtouch_set_rate(PyObject* self, PyObject* args) {
  float rate;

  // Given the rate as a fraction
  if (!PyArg_ParseTuple(args, "f", &rate) || rate < 0) {
    PyErr_SetString(PyExc_TypeError, "invalid argument");
	return NULL;
  }

  PY_SOUNDTOUCH(self)->soundtouch->setRate(rate);

  Py_INCREF(Py_None);
  return Py_None;
}

// Set the tempo of playback
static PyObject* py_soundtouch_set_tempo(PyObject* self, PyObject* args) {
  float tempo;

  // Given the tempo as a ratio
  if (!PyArg_ParseTuple(args, "f", &tempo) || tempo < 0) {
    PyErr_SetString(PyExc_TypeError, "invalid argument");
	return NULL;
  }

  PY_SOUNDTOUCH(self)->soundtouch->setTempo(tempo);

  Py_INCREF(Py_None);
  return Py_None;
}

// Shift all pitches
static PyObject* py_soundtouch_set_pitch(PyObject* self, PyObject* args) {
  float pitch;

  // Given the pitch adjustment as a fraction
  if (!PyArg_ParseTuple(args, "f", &pitch) || pitch < 0) {
    PyErr_SetString(PyExc_TypeError, "invalid argument");
	return NULL;
  }

  PY_SOUNDTOUCH(self)->soundtouch->setPitch(pitch);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject* py_soundtouch_set_pitch_shift(PyObject* self, PyObject* args) {
  float pitch;

  // Given the pitch adjustment in half-steps
  if (!PyArg_ParseTuple(args, "f", &pitch) || pitch < -12 || pitch > 12) {
    PyErr_SetString(PyExc_TypeError, "invalid argument: pitch must be between -12 and 12");
	return NULL;
  }

  PY_SOUNDTOUCH(self)->soundtouch->setPitchSemiTones(pitch);

  Py_INCREF(Py_None);
  return Py_None;
}

// Flush all samples to the output
static PyObject* py_soundtouch_flush(PyObject* self, PyObject* args) {
  PY_SOUNDTOUCH(self)->soundtouch->flush();

  // For debugging:
  //delete inputs;
  //delete outputs;

  Py_INCREF(Py_None);
  return Py_None;
}

// Clear all buffers
static PyObject* py_soundtouch_clear(PyObject* self, PyObject* args) {
  PY_SOUNDTOUCH(self)->soundtouch->clear();

  Py_INCREF(Py_None);
  return Py_None;
}

// Add new samples
static PyObject* py_soundtouch_put_samples(PyObject* self, PyObject* args) {
  py_soundtouch* ps = PY_SOUNDTOUCH(self);
  int buflen;
  const char* transfer;

  // Given a string of samples to add
  if (!PyArg_ParseTuple(args, "y#", &transfer, &buflen)) {
    PyErr_SetString(PyExc_TypeError, "invalid argument");
    return NULL;
  }

  // Move them into our char-short union
  /*
  for (int ii = 0; ii < buflen; ii++)
    ps->buffer.chars[ii] = transfer[ii];
  */

  // Add them
  //ps->soundtouch->putSamples(ps->buffer.shorts, (uint) buflen / (2 * ps->channels));
  ps->soundtouch->putSamples((const soundtouch::SAMPLETYPE*)transfer, (uint) buflen / (2 * ps->channels));

  // For debugging:
  //inputs->write(ps->buffer.shorts, buflen / 2);

  Py_INCREF(Py_None);
  return Py_None;
}

// Return the shifted samples
static PyObject* py_soundtouch_get_samples(PyObject* self, PyObject* args) {
  py_soundtouch* ps = PY_SOUNDTOUCH(self);
  uint maxSamples;

  // Given the number of samples to request
  if (!PyArg_ParseTuple(args, "|I", &maxSamples)) {
    PyErr_SetString(PyExc_TypeError, "invalid argument");
	return NULL;
  }
  
  std::vector<char> buf( maxSamples * sizeof( soundtouch::SAMPLETYPE ) * ps->channels );
  // Move them into our char-short union
  uint received = ps->soundtouch->receiveSamples((soundtouch::SAMPLETYPE*)buf.data(), maxSamples);

  // For debugging:
  //outputs->write(ps->buffer.shorts, received * ps->channels);

  return PyBytes_FromStringAndSize(buf.data(), received * sizeof( soundtouch::SAMPLETYPE ) * ps->channels);
}

// Return how many samples are available for output
static PyObject* py_soundtouch_ready_count(PyObject* self, PyObject* args) {
  return PyLong_FromLong(PY_SOUNDTOUCH(self)->soundtouch->numSamples());
}

// Return how many samples will be available given the current data
static PyObject* py_soundtouch_waiting_count(PyObject* self, PyObject* args) {
  return PyLong_FromLong(PY_SOUNDTOUCH(self)->soundtouch->numUnprocessedSamples());
}

static PyObject* py_soundtouch_set_sample_rate(PyObject* self, PyObject* args) {
  int rate;

  // Given the rate as a fraction
  if (!PyArg_ParseTuple(args, "i", &rate) || rate < 0) {
    PyErr_SetString(PyExc_TypeError, "invalid argument");
	return NULL;
  }

  PY_SOUNDTOUCH(self)->soundtouch->setSampleRate(rate);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject* py_soundtouch_set_channels(PyObject* self, PyObject* args) {
  unsigned int channels;

  // Given the rate as a fraction
  if (!PyArg_ParseTuple(args, "i", &channels) || channels < 0) {
    PyErr_SetString(PyExc_TypeError, "invalid argument");
	return NULL;
  }

  PY_SOUNDTOUCH(self)->soundtouch->setChannels(channels);

  Py_INCREF(Py_None);
  return Py_None;
}

/* housekeeping */

static PyMethodDef soundtouch_methods[] = {
    { "set_rate", py_soundtouch_set_rate, METH_VARARGS, "" },
    { "set_tempo", py_soundtouch_set_tempo, METH_VARARGS, "" },
    { "set_pitch", py_soundtouch_set_pitch, METH_VARARGS, "" },
    { "set_pitch_shift", py_soundtouch_set_pitch_shift, METH_VARARGS, "" },
    { "flush", py_soundtouch_flush, METH_VARARGS, "" },
    { "clear", py_soundtouch_clear, METH_VARARGS, "" },
    { "put_samples", py_soundtouch_put_samples, METH_VARARGS, "" },
    { "get_samples", py_soundtouch_get_samples, METH_VARARGS, "" },
    { "ready_count", py_soundtouch_ready_count, METH_VARARGS, "" },
    { "waiting_count", py_soundtouch_waiting_count, METH_VARARGS, "" },
    { "set_sample_rate", py_soundtouch_set_sample_rate, METH_VARARGS, "" },
    { "set_channels", py_soundtouch_set_channels, METH_VARARGS, "" },
    { NULL }
};

PyTypeObject py_soundtouch_t = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "SoundTouch",                   /* tp_name */
    sizeof(py_soundtouch),          /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor) py_soundtouch_dealloc,      /* tp_dealloc */
    0,                              /* tp_vectorcall_offset */
    (getattrfunc) py_soundtouch_getattr,     /* tp_getattr */
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
    "SoundTouch",                   /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
    soundtouch_methods,             /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    py_soundtouch_init,             /* tp_init */
    0,                              /* tp_alloc */
    PyType_GenericNew,              /* tp_new */
};

// Get additional attributes from the SoundTouch object
static PyObject* py_soundtouch_getattr(PyObject* self, char* name) {
  // TODO: add soundtouch.getSetting here?  Add a setattr with soundtouch.setSetting?
  //return Py_FindMethod(soundtouch_methods, self, name);
  return PyObject_GenericGetAttr((PyObject *)self, PyUnicode_FromString(name));
}
