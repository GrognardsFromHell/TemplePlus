
#pragma once
#include "../tio/tio.h"

PyObject *PyTioFile_FromTioFile(TioFile *file);
void PyTioFile_Invalidate(PyObject *pyTioFile);
