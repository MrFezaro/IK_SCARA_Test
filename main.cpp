#include "GUI.hpp"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    SerialComm motorA("COM8");
    SerialComm motorB("COM9");

    motorA.setNumPolePairs(15);
    motorA.setCurrentLimit(10000);
    motorA.setPositionKp(10.0);
    motorA.setVelocityKp(1.0);
    motorA.setDrivingMode(Position);

    motorB.setNumPolePairs(15);
    motorB.setCurrentLimit(10000);
    motorB.setPositionKp(10.0);
    motorB.setVelocityKp(1.0);
    motorB.setDrivingMode(Position);

    setMotors(&motorA, &motorB);

    WNDCLASSA wc     = {};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = "FiveBarIK";
    wc.hCursor       = LoadCursor(NULL, IDC_CROSS);
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowA("FiveBarIK", "Five-bar IK",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600, NULL, NULL, hInst, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}