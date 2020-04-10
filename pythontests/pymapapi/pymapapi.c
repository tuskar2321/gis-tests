#define PY_SSIZE_T_CLEAN
#include <Python.h>

static PyObject *pyMapapi_Error;
static struct PyModuleDef pyMapapi_module;

PyMODINIT_FUNC PyInit_pymapapi(void) {
  PyObject *m;

  m = PyModule_Create(&pyMapapi_module);
  if (m == NULL)
    return NULL;

  pyMapapi_Error = PyErr_NewException("pymapapi.error", NULL, NULL);
  Py_INCREF(pyMapapi_Error);
  PyModule_AddObject(m, "error", pyMapapi_Error);
  return m;
}

int IsLittleEndian() {
  unsigned short temp = 1;
  return (*(unsigned char *)(&temp) == 1);
}

#include "iconv.h"
void pyMapapi_Unicode8ToUnicode(const char *src, unsigned short *dst,
                                size_t dstSize) {
  char *instring = (char *)src;
  size_t inbytes = strlen(src);
  char *outstring = (char *)dst;
  size_t outbytes = dstSize;

  iconv_t codec;
  if (IsLittleEndian() == 1)
    codec = iconv_open("UTF-16LE", "UTF-8");
  else
    codec = iconv_open("UTF-16BE", "UTF-8");

  iconv(codec, &instring, &inbytes, &outstring, &outbytes);
  iconv_close(codec);
}

// Calculate length of string WCHAR
long int pyMapapi_strlen(const unsigned short *string) {
  long int size = 0;
  while (*string != 0) {
    size++;
    string++;
  }
  return size;
}

//----------------------------------------------------------------------
// Global definitions for MAPAPI
typedef unsigned short WCHAR;
typedef void *HMAP;

//----------------------------------------------------------------------
long int mapMessageEnable(long int enable);

static PyObject *pyMapapi_mapMessageEnable(PyObject *self, PyObject *args) {
  long int returnValue = 0;
  long int enableMessages = 0;

  if (!PyArg_ParseTuple(args, "l", &enableMessages))
    return NULL;

  returnValue = mapMessageEnable(enableMessages);
  return PyLong_FromLong(returnValue);
}

//----------------------------------------------------------------------

long int mapOpenConnectUn(const WCHAR *name, long int port);

static PyObject *pyMapapi_mapOpenConnectUn(PyObject *self, PyObject *args) {
  const char *connectString;
  unsigned short connectStringUn[2048] = {0};
  long int portNumber = 0;
  long int returnValue = 0;

  if (!PyArg_ParseTuple(args, "sl", &connectString, &portNumber))
    return NULL;

  pyMapapi_Unicode8ToUnicode(connectString, connectStringUn,
                             sizeof(connectStringUn));
  returnValue = mapOpenConnectUn(connectStringUn, portNumber);
  if (returnValue <= 0) {
    fprintf(stderr, "0 = %d, 1 = %d\n", connectStringUn[0], connectStringUn[1]);
    fprintf(stderr, "Address = %s, Port = %ld\n", connectString, portNumber);
    PyErr_SetString(pyMapapi_Error, "Open connection error");
    return NULL;
  }
  return PyLong_FromLong(returnValue);
}

//----------------------------------------------------------------------
long int mapCloseConnect(long int number);

static PyObject *pyMapapi_mapCloseConnect(PyObject *self, PyObject *args) {
  long int connectNumber = 0;
  long int returnValue = 0;

  if (!PyArg_ParseTuple(args, "l", &connectNumber))
    return NULL;

  returnValue = mapCloseConnect(connectNumber);
  if (returnValue == 0) {
    PyErr_SetString(pyMapapi_Error, "Connection not found");
    return NULL;
  }
  if (returnValue <= 0) {
    PyErr_SetString(pyMapapi_Error, "Connection is busy");
    return NULL;
  }
  return PyLong_FromLong(returnValue);
}

//----------------------------------------------------------------------
typedef struct TMCUSERPARM {
  char Name[32];
  char Password[64];
} TMCUSERPARM;

long int mapRegisterUserEx(long int number, TMCUSERPARM *parm);

static PyObject *pyMapapi_mapRegisterUserEx(PyObject *self, PyObject *args) {
  long int connectionNumber;
  const char *userName;
  const char *userPassword;
  int returnValue = 0;

  if (!PyArg_ParseTuple(args, "lss", &connectionNumber, &userName,
                        &userPassword))
    return NULL;

  TMCUSERPARM registrationParameters;
  strncpy(registrationParameters.Name, userName,
          sizeof(registrationParameters.Name));
  strncpy(registrationParameters.Password, userPassword,
          sizeof(registrationParameters.Password));

  returnValue = mapRegisterUserEx(connectionNumber, &registrationParameters);

  if (returnValue <= 0) {
    PyErr_SetString(pyMapapi_Error, "User registration error");
    return NULL;
  }
  return PyLong_FromLong(returnValue);
}

//----------------------------------------------------------------------

HMAP mapOpenAnyData(const WCHAR *name, long int mode, long int *error);

static PyObject *pyMapapi_mapOpenAnyData(PyObject *self, PyObject *args) {
  char *openPath;
  long int openMode;
  long int openError;
  unsigned short openPathUn[2048] = {0};

  if (!PyArg_ParseTuple(args, "sl", &openPath, &openMode))
    return NULL;

  pyMapapi_Unicode8ToUnicode(openPath, openPathUn, sizeof(openPathUn));

  HMAP mapHandle = mapOpenAnyData(openPathUn, openMode, &openError);

  if (mapHandle == NULL) {
    char errorMessage[1024] = {0};
    sprintf(errorMessage, "Error open data: %ld", openError);
    PyErr_SetString(pyMapapi_Error, errorMessage);
    return NULL;
  }
  return PyLong_FromVoidPtr(mapHandle);
}

//----------------------------------------------------------------------

HMAP mapOpenAnyDataPro(const WCHAR *name, long int mode, long int *error,
                       const WCHAR *password, long int size);

static PyObject *pyMapapi_mapOpenAnyDataPro(PyObject *self, PyObject *args) {
  char *openPath;
  char *password;
  long int openMode;
  long int openError;
  unsigned short openPathUn[2048] = {0};
  unsigned short passwordUn[2048] = {0};

  if (!PyArg_ParseTuple(args, "sls", &openPath, &openMode, &password))
    return NULL;

  pyMapapi_Unicode8ToUnicode(openPath, openPathUn, sizeof(openPathUn));
  pyMapapi_Unicode8ToUnicode(password, passwordUn, sizeof(passwordUn));
  long int passwordLen = pyMapapi_strlen(passwordUn) * sizeof(passwordUn[0]);

  HMAP mapHandle = mapOpenAnyDataPro(openPathUn, openMode, &openError,
                                     passwordUn, passwordLen);

  if (mapHandle == NULL) {
    char errorMessage[1024] = {0};
    fprintf(stderr, "Password = %s\n", password);
    sprintf(errorMessage, "Error open data: %ld", openError);
    PyErr_SetString(pyMapapi_Error, errorMessage);
    return NULL;
  }
  return PyLong_FromVoidPtr(mapHandle);
}

//----------------------------------------------------------------------

void mapCloseData(HMAP mapHandle);

static PyObject *pyMapapi_mapCloseData(PyObject *self, PyObject *args) {
  off_t returnValue = 0; // off_t can store pointer
  long int mapHandle;

  if (!PyArg_ParseTuple(args, "l", &mapHandle))
    return NULL;

  mapCloseData((HMAP)mapHandle);
  return PyLong_FromLong(returnValue);
}

//----------------------------------------------------------------------

long int mapGetObjectCount(HMAP mapHandle, long int listNumber);

static PyObject *pyMapapi_mapGetObjectCount(PyObject *self, PyObject *args) {
  long int returnValue = 0;
  long int mapHandle;
  long int listNumber;

  if (!PyArg_ParseTuple(args, "ll", &mapHandle, &listNumber))
    return NULL;

  returnValue = mapGetObjectCount((HMAP)mapHandle, listNumber);

  if (returnValue == 0) {
    PyErr_SetString(pyMapapi_Error, "Error count objects");
    return NULL;
  }
  return PyLong_FromLong(returnValue);
}

//----------------------------------------------------------------------

void mapAdjustData(HMAP mapHandle);

static PyObject *pyMapapi_mapAdjustData(PyObject *self, PyObject *args) {
  off_t returnValue = 0; // off_t can store pointer
  long int mapHandle;

  if (!PyArg_ParseTuple(args, "l", &mapHandle))
    return NULL;

  mapAdjustData((HMAP)mapHandle);
  return PyLong_FromLong(returnValue);
}

//----------------------------------------------------------------------

static PyMethodDef pyMapapiMethods[] = {
    {"mapMessageEnable", pyMapapi_mapMessageEnable, METH_VARARGS,
     "Enable message-boxes from mapapi."},
    {"mapOpenConnectUn", pyMapapi_mapOpenConnectUn, METH_VARARGS,
     "Open connection with GISServer."},
    {"mapCloseConnect", pyMapapi_mapCloseConnect, METH_VARARGS,
     "Close connection with GISServer."},
    {"mapRegisterUserEx", pyMapapi_mapRegisterUserEx, METH_VARARGS,
     "Register user on GISServer."},
    {"mapOpenAnyData", pyMapapi_mapOpenAnyData, METH_VARARGS, "Open data."},
    {"mapOpenAnyDataPro", pyMapapi_mapOpenAnyDataPro, METH_VARARGS,
     "Open data with password(sitx)."},
    {"mapCloseData", pyMapapi_mapCloseData, METH_VARARGS, "Close data."},
    {"mapGetObjectCount", pyMapapi_mapGetObjectCount, METH_VARARGS,
     "Get object count."},
    {"mapAdjustData", pyMapapi_mapAdjustData, METH_VARARGS, "Adjust data."},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static struct PyModuleDef pyMapapi_module = {
    PyModuleDef_HEAD_INIT, "pymapapi", /* name of module */
    NULL,                              /* module documentation, may be NULL */
    -1, /* size of per-interpreter state of the module,
         or -1 if the module keeps state in global variables. */
    pyMapapiMethods};
