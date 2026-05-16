#ifndef GUI_HPP
#define GUI_HPP

#include <windows.h>
#include "SerialComm/SerialComm.hpp"

void setMotors(SerialComm* a, SerialComm* b);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

#endif