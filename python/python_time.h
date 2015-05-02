
#pragma once

struct GameTime;

PyObject *PyTimeStamp_Create();
PyObject *PyTimeStamp_Create(const GameTime &gameTime);
