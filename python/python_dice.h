
#pragma once

class Dice;

extern PyTypeObject PyDiceType;
PyObject *PyDice_FromDice(const Dice &dice);
