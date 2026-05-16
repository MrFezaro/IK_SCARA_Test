#include <iostream>

#include "GUI.hpp"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    SerialComm motorA("COM8");
    SerialComm motorB("COM9");

    motorA.setNumPolePairs(15);
    motorA.setCurrentLimit(10000);
    motorA.setPositionKp(15.0);
    motorA.setVelocityKp(1.0);
    motorA.setDrivingMode(Position);
    motorA.setTorqueSign(-1.0f);

    motorB.setNumPolePairs(15);
    motorB.setCurrentLimit(10000);
    motorB.setPositionKp(15.0);
    motorB.setVelocityKp(1.0);
    motorB.setDrivingMode(Position);
    motorB.setTorqueSign(-1.0f);

    setMotors(&motorA, &motorB);

    // Zero both motors at startup
    motorA.setPositionSetpoint(0.0f);
    motorB.setPositionSetpoint(0.0f);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

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

// int main() {
//     SerialComm serialComm("COM8"); // Probably "/dev/ttyACM0" for linux.
//
//     // Adjust the following values as wanted.
//     serialComm.setNumPolePairs(15);
//     serialComm.setCurrentLimit(10000);
//     serialComm.setDrivingMode(DrivingMode::OpenLoop); // For velocity, change this to "DrivingMode::Velocity"
//
//     SensorData data;
//     for (int i = 0; i < 1000; i++) {
//         serialComm.getData(data);
//         std::cout << data.timestamp_ms << " | Position: " << data.position << " | Velocity: " << data.velocity << " | Current: " << data.current << std::endl;
//
//         if (serialComm.getNumRemainingCommands() < 3) {
//             // serialComm.setPositionSetpoint(static_cast<float>(i)); // For velocity, change this to ".setVelocitySetpoint"
//             serialComm.setVelocitySetpoint(10.0);
//         }
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }
// }