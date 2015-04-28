
#pragma once

struct TioFile;
struct GameSystemSaveFile;

PyObject *PyGame_Create();
void PyGame_Reset();
void PyGame_Exit();
bool PyGame_Save(TioFile *file);
bool PyGame_Load(GameSystemSaveFile *file);
