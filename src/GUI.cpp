#include "GUI.hpp"
#include "IK.hpp"
#include <cmath>
#include <cstdio>
#include <vector>
#include <optional>

using namespace std;

static std::vector<float> baseA, baseB, target;
static float linkLen = 150.f;
static bool hasTarget = false;

static void initBases(int W, int H) {
    baseA = {W/2.f - 80.f, H/2.f + 60.f};
    baseB = {W/2.f + 80.f, H/2.f + 60.f};
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

static void drawText(HDC hdc, int x, int y, const char* fmt, float val) {
    char buf[64];
    snprintf(buf, sizeof(buf), fmt, val);
    TextOutA(hdc, x, y, buf, (int)strlen(buf));
}

static void drawText2(HDC hdc, int x, int y, const char* fmt, float a, float b) {
    char buf[64];
    snprintf(buf, sizeof(buf), fmt, a, b);
    TextOutA(hdc, x, y, buf, (int)strlen(buf));
}

static void drawInfoPanel(HDC hdc, int W, int H, const IKResult& ik, const std::vector<float>& origin) {
    // world coords: origin is midpoint between base joints
    float wx = target[0] - origin[0];
    float wy = -(target[1] - origin[1]); // flip Y so up is positive

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(55, 138, 221));
    drawText (hdc, 16, 16, "theta1:  %.1f deg", ik.theta1);
    drawText2(hdc, 16, 36, "  joint A:  (%.1f, %.1f) px", ik.jointA[0], ik.jointA[1]);

    SetTextColor(hdc, RGB(212, 83, 126));
    drawText (hdc, 16, 62, "theta2:  %.1f deg", ik.theta2);
    drawText2(hdc, 16, 82, "  joint B:  (%.1f, %.1f) px", ik.jointB[0], ik.jointB[1]);

    SetTextColor(hdc, RGB(60, 60, 60));
    drawText2(hdc, 16, 108, "end-effector:  (%.1f, %.1f) px", wx, wy);

    char hint[64];
    snprintf(hint, sizeof(hint), "link length: %.0f  (up/down to adjust)", linkLen);
    SetTextColor(hdc, RGB(150, 150, 150));
    TextOutA(hdc, 16, H-28, hint, (int)strlen(hint));
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
        TextOutA(hdc, W/2-100, (int)baseA[1]-80, "click to place end-effector", 27);
        return;
    }

    std::vector<float> origin = {(baseA[0] + baseB[0]) / 2.f, baseA[1]};
    auto result = fiveBarIK(baseA, baseB, target, linkLen);

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

        drawInfoPanel(hdc, W, H, *result, origin);
    } else {
        HPEN rp = CreatePen(PS_SOLID, 2, RGB(220, 50, 50));
        SelectObject(hdc, rp);
        MoveToEx(hdc, (int)target[0]-10, (int)target[1]-10, NULL); LineTo(hdc, (int)target[0]+10, (int)target[1]+10);
        MoveToEx(hdc, (int)target[0]+10, (int)target[1]-10, NULL); LineTo(hdc, (int)target[0]-10, (int)target[1]+10);
        DeleteObject(rp);
        SetTextColor(hdc, RGB(220, 50, 50));
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, 16, 16, "unreachable", 11);
    }

    drawBaseJoint(hdc, baseA, "A");
    drawBaseJoint(hdc, baseB, "B");
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    static int W = 800, H = 600;

    switch (msg) {
    case WM_CREATE:
        initBases(W, H);
        target = {W/2.f, H/2.f - 80.f};
        break;

    case WM_SIZE:
        W = LOWORD(lp); H = HIWORD(lp);
        initBases(W, H);
        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:
        if (msg == WM_MOUSEMOVE && !(wp & MK_LBUTTON)) break;
        target = {(float)LOWORD(lp), (float)HIWORD(lp)};
        hasTarget = true;
        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_KEYDOWN:
        if (wp == VK_UP)   { linkLen = min(linkLen + 5.f, 250.f); InvalidateRect(hwnd, NULL, FALSE); }
        if (wp == VK_DOWN) { linkLen = max(linkLen - 5.f,  50.f); InvalidateRect(hwnd, NULL, FALSE); }
        break;

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
        PostQuitMessage(0);
        break;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}