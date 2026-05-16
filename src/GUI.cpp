#include "gui.hpp"
#include "ik.hpp"
#include "animation.hpp"
#include <cmath>
#include <cstdio>
#include <vector>
#include <optional>
#include <string>
#include <chrono>
#include "SerialComm/SerialComm.hpp"

static SerialComm* motorA = nullptr;
static SerialComm* motorB = nullptr;

void setMotors(SerialComm* a, SerialComm* b) {
    motorA = a;
    motorB = b;
}

static std::vector<float> baseA, baseB;
static std::vector<float> target;       // current real (possibly blending) position
static std::vector<float> manualTarget; // destination when shape == None
static bool               hasTarget = false;
static float              pxPerCm   = 3.5f;
static AnimState          anim;
static std::chrono::steady_clock::time_point lastTick;

static constexpr float DEG_TO_RAD = 3.14159265f / 180.f;

static void sendMotorAngles(const IKResult& ik) {
    if (!motorA || !motorB) return;

    // encoder 0 = upright = 90 deg in our coord system
    // so offset by -90 deg before sending
    float setpointA = (ik.theta1 - 90.f) * DEG_TO_RAD;
    float setpointB = (ik.theta2 - 90.f) * DEG_TO_RAD;

    if (motorA->getNumRemainingCommands() < 3)
        motorA->setPositionSetpoint(setpointA);
    if (motorB->getNumRemainingCommands() < 3)
        motorB->setPositionSetpoint(setpointB);

    std::this_thread::sleep_for(std::chrono::milliseconds(25));
}

static void initBases(int W, int H) {
    float halfBase = (BASE_SEPARATION * pxPerCm) / 2.f;
    baseA = {W/2.f - halfBase, H/2.f + 60.f};
    baseB = {W/2.f + halfBase, H/2.f + 60.f};
}

static std::vector<float> worldOrigin() {
    return { (baseA[0] + baseB[0]) / 2.f, baseA[1] };
}

static void drawLine(HDC hdc, const std::vector<float>& a, const std::vector<float>& b) {
    MoveToEx(hdc, (int)a[0], (int)a[1], NULL);
    LineTo  (hdc, (int)b[0], (int)b[1]);
}

static void drawCircle(HDC hdc, const std::vector<float>& p, int r) {
    Ellipse(hdc, (int)p[0]-r, (int)p[1]-r, (int)p[0]+r, (int)p[1]+r);
}

static void drawArm(HDC hdc,
    const std::vector<float>& base,
    const std::vector<float>& joint,
    const std::vector<float>& tip,
    COLORREF col, COLORREF dimCol)
{
    HPEN p1 = CreatePen(PS_SOLID, 4, col);
    SelectObject(hdc, p1);
    drawLine(hdc, base, joint);
    DeleteObject(p1);

    HPEN p2 = CreatePen(PS_SOLID, 4, dimCol);
    SelectObject(hdc, p2);
    drawLine(hdc, joint, tip);
    DeleteObject(p2);

    HBRUSH wb = CreateSolidBrush(RGB(255, 255, 255));
    HPEN   wp = CreatePen(PS_SOLID, 2, col);
    SelectObject(hdc, wb);
    SelectObject(hdc, wp);
    drawCircle(hdc, joint, 6);
    DeleteObject(wb);
    DeleteObject(wp);
}

static void drawBaseJoint(HDC hdc, const std::vector<float>& p, const char* label) {
    HBRUSH b  = CreateSolidBrush(RGB(60, 60, 60));
    HPEN   pn = CreatePen(PS_SOLID, 1, RGB(60, 60, 60));
    SelectObject(hdc, b);
    SelectObject(hdc, pn);
    drawCircle(hdc, p, 9);
    DeleteObject(b);
    DeleteObject(pn);
    SetTextColor(hdc, RGB(80, 80, 80));
    SetBkMode(hdc, TRANSPARENT);
    TextOutA(hdc, (int)p[0]-4, (int)p[1]+12, label, 1);
}

static void drawInfoPanel(HDC hdc, int H, const IKResult& ik) {
    auto origin = worldOrigin();
    float wx = (target[0] - origin[0]) / pxPerCm;
    float wy = -((target[1] - origin[1]) / pxPerCm);

    SetBkMode(hdc, TRANSPARENT);
    char buf[128];

    // chain A — blue
    SetTextColor(hdc, RGB(55, 138, 221));
    snprintf(buf, sizeof(buf), "theta1:  %.1f deg", ik.theta1);
    TextOutA(hdc, 16, 16, buf, (int)strlen(buf));
    snprintf(buf, sizeof(buf), "  phi1 (distal A):  %.1f deg", ik.phi1);
    TextOutA(hdc, 16, 36, buf, (int)strlen(buf));

    // chain B — pink
    SetTextColor(hdc, RGB(212, 83, 126));
    snprintf(buf, sizeof(buf), "theta2:  %.1f deg", ik.theta2);
    TextOutA(hdc, 16, 62, buf, (int)strlen(buf));
    snprintf(buf, sizeof(buf), "  phi2 (distal B):  %.1f deg", ik.phi2);
    TextOutA(hdc, 16, 82, buf, (int)strlen(buf));

    // end-effector
    SetTextColor(hdc, RGB(60, 60, 60));
    snprintf(buf, sizeof(buf), "end-effector:  (%.2f cm, %.2f cm)", wx, wy);
    TextOutA(hdc, 16, 108, buf, (int)strlen(buf));
    snprintf(buf, sizeof(buf), "ee-line dist:  %.2f px", ik.eeDist);
    TextOutA(hdc, 16, 128, buf, (int)strlen(buf));

    // animation label
    if (anim.shape != AnimShape::None) {
        SetTextColor(hdc, RGB(100, 160, 100));
        char abuf[64];
        snprintf(abuf, sizeof(abuf), "anim: %s  |  speed: %.1f  (left/right)",
                 shapeName(anim.shape).c_str(), anim.speed);
        TextOutA(hdc, 16, H-50, abuf, (int)strlen(abuf));
    }

    // bottom hint
    SetTextColor(hdc, RGB(150, 150, 150));
    snprintf(buf, sizeof(buf), "L1=%.1fcm  L2=%.1fcm  |  up/down = zoom  |  scale=%.2f px/cm",
             L1, L2, pxPerCm);
    TextOutA(hdc, 16, H-28, buf, (int)strlen(buf));
}

static void drawScene(HDC hdc, int W, int H) {
    HBRUSH bg = CreateSolidBrush(RGB(245, 245, 243));
    RECT rc = {0, 0, W, H};
    FillRect(hdc, &rc, bg);
    DeleteObject(bg);

    HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(210, 210, 210));
    SelectObject(hdc, gridPen);
    MoveToEx(hdc, 0, (int)baseA[1], NULL); LineTo(hdc, W, (int)baseA[1]);
    MoveToEx(hdc, W/2, 0, NULL);           LineTo(hdc, W/2, H);
    DeleteObject(gridPen);

    if (!hasTarget) {
        drawBaseJoint(hdc, baseA, "A");
        drawBaseJoint(hdc, baseB, "B");
        SetTextColor(hdc, RGB(150, 150, 150));
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, W/2-140, (int)baseA[1]-80,
                 "click to place  |  1-7 = animation  |  0 = stop", 46);
        return;
    }

    auto result = fiveBarIK(baseA, baseB, target, L1 * pxPerCm, L2 * pxPerCm);

    if (result) {
        drawArm(hdc, baseA, result->jointA, target, RGB(55,138,221), RGB(130,185,235));
        drawArm(hdc, baseB, result->jointB, target, RGB(212,83,126), RGB(230,155,185));

        HBRUSH eb = CreateSolidBrush(RGB(55, 138, 221));
        HPEN   ep = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
        SelectObject(hdc, eb);
        SelectObject(hdc, ep);
        drawCircle(hdc, target, 8);
        DeleteObject(eb);
        DeleteObject(ep);

        sendMotorAngles(*result);
        drawInfoPanel(hdc, H, *result);
    } else {
        HPEN rp = CreatePen(PS_SOLID, 2, RGB(220, 50, 50));
        SelectObject(hdc, rp);
        MoveToEx(hdc, (int)target[0]-10, (int)target[1]-10, NULL); LineTo(hdc, (int)target[0]+10, (int)target[1]+10);
        MoveToEx(hdc, (int)target[0]+10, (int)target[1]-10, NULL); LineTo(hdc, (int)target[0]-10, (int)target[1]+10);
        DeleteObject(rp);
        SetTextColor(hdc, RGB(220, 50, 50));
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, 16, 16, "unreachable or outside joint limits", 35);

        char buf[128];
        SetTextColor(hdc, RGB(150, 150, 150));
        snprintf(buf, sizeof(buf), "L1=%.1fcm  L2=%.1fcm  |  up/down = zoom  |  scale=%.2f px/cm",
                 L1, L2, pxPerCm);
        TextOutA(hdc, 16, H-28, buf, (int)strlen(buf));
    }

    drawBaseJoint(hdc, baseA, "A");
    drawBaseJoint(hdc, baseB, "B");
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    static int W = 800, H = 600;

    switch (msg) {
    case WM_CREATE:
        initBases(W, H);
        target       = {W/2.f, H/2.f - 80.f};
        manualTarget = target;
        lastTick     = std::chrono::steady_clock::now();
        SetTimer(hwnd, 1, 16, NULL);
        break;

    case WM_SIZE:
        W = LOWORD(lp); H = HIWORD(lp);
        initBases(W, H);
        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:
        if (msg == WM_MOUSEMOVE && !(wp & MK_LBUTTON)) break;
        {
            // Treat manual clicks exactly like startAnim: capture current
            // blended position as fromPos and ease toward the new destination.
            anim.fromPos  = target;   // where the arm actually is right now
            anim.blendT   = 0.f;      // restart the ease-in
            anim.shape    = AnimShape::None;
            manualTarget  = {(float)LOWORD(lp), (float)HIWORD(lp)};
            hasTarget     = true;
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;

    case WM_KEYDOWN:
        if (wp == VK_UP)    { pxPerCm = std::min(pxPerCm + 0.25f, 8.f);    initBases(W, H); InvalidateRect(hwnd, NULL, FALSE); }
        if (wp == VK_DOWN)  { pxPerCm = std::max(pxPerCm - 0.25f, 0.25f);  initBases(W, H); InvalidateRect(hwnd, NULL, FALSE); }
        if (wp == VK_RIGHT) { anim.speed = std::min(anim.speed + 0.05f, 5.f);   InvalidateRect(hwnd, NULL, FALSE); }
        if (wp == VK_LEFT)  { anim.speed = std::max(anim.speed - 0.05f, 0.05f); InvalidateRect(hwnd, NULL, FALSE); }
        if (wp == '0') { anim.shape = AnimShape::None;                           InvalidateRect(hwnd, NULL, FALSE); }
        if (wp == '1') { startAnim(anim, AnimShape::LineH,    target); hasTarget = true; }
        if (wp == '2') { startAnim(anim, AnimShape::LineV,    target); hasTarget = true; }
        if (wp == '3') { startAnim(anim, AnimShape::Square,   target); hasTarget = true; }
        if (wp == '4') { startAnim(anim, AnimShape::Triangle, target); hasTarget = true; }
        if (wp == '5') { startAnim(anim, AnimShape::Circle,   target); hasTarget = true; }
        if (wp == '6') { startAnim(anim, AnimShape::Figure8,  target); hasTarget = true; }
        if (wp == '7') { startAnim(anim, AnimShape::Heart,    target); hasTarget = true; }
        break;

    case WM_TIMER: {
        auto  now = std::chrono::steady_clock::now();
        float dt  = std::chrono::duration<float>(now - lastTick).count();
        lastTick  = now;

        if (anim.shape != AnimShape::None) {
            // Animation driving: animStep owns the full blend + movement.
            target = animStep(anim, dt, worldOrigin(), pxPerCm);
            InvalidateRect(hwnd, NULL, FALSE);
        } else if (anim.blendT < 1.f) {
            // Manual target: ease from fromPos toward manualTarget using the
            // same blendSpeed and smoothStep as animation transitions.
            anim.blendT = fminf(1.f, anim.blendT + anim.blendSpeed * dt);
            float blend = smoothStep(anim.blendT);
            target = {
                anim.fromPos[0] + blend * (manualTarget[0] - anim.fromPos[0]),
                anim.fromPos[1] + blend * (manualTarget[1] - anim.fromPos[1])
            };
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, W, H);
        SelectObject(memDC, bmp);
        drawScene(memDC, W, H);
        BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
        DeleteObject(bmp);
        DeleteDC(memDC);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_DESTROY:
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        break;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}