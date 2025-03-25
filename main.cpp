#include "DXManager.h"
#include "LogManager.h"

DXManager DXManInstance;
LogManager logInstance;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

bool isMouseDown = false;
POINT lastMousePos;

void RedirectIOToConsole() {
    AllocConsole();

    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    setvbuf(stdout, nullptr, _IONBF, 0);

    freopen_s(&fp, "CONOUT$", "w", stderr);
    setvbuf(stderr, nullptr, _IONBF, 0);

    freopen_s(&fp, "CONIN$", "r", stdin);
    setvbuf(stdin, nullptr, _IONBF, 0);

    std::ios::sync_with_stdio();
}

float yaw = 0.0f;
float pitch = 0.0f;
const float sensitivity = 0.001f;
float cameraSpeed = 200.0f;
bool keyWPressed = false;
bool keySPressed = false;
bool keyAPressed = false;
bool keyDPressed = false;
bool isShiftPressed = false;
bool keyEPressed = false;
bool keyQPressed = false;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    logInstance.InitializeLog();

    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "WindowClass";
    RegisterClassEx(&wc);

    RECT wr = { 0, 0, (LONG)DXManInstance.resW, (LONG)DXManInstance.resH };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    DXManInstance.hWnd = CreateWindowEx(NULL, "WindowClass", "PEngine", WS_OVERLAPPEDWINDOW,
        300, 200, wr.right - wr.left, wr.bottom - wr.top,
        nullptr, nullptr, hInstance, nullptr);

    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.dwFlags = 0;
    rid.hwndTarget = nullptr;

    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
        LogManager::pPrint(true, true,"Failed to register raw input devices.");
        return -1;
    }

    RedirectIOToConsole();

    ShowWindow(DXManInstance.hWnd, nCmdShow);

    if (!DXManInstance.InitDirect3D("Assets/sponza.obj")) {
        MessageBox(0, "Direct3D Initialization - Failed", "Error", MB_OK);
        return 0;
    }

    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));

    int frameCount = 0;

    XMVECTOR cameraPosition = XMVectorSet(0.0f, 10.0f, -40.0f, 0.0f);

    auto start = std::chrono::high_resolution_clock::now();

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            if (isMouseDown) {
                SetCursorPos((int)DXManInstance.resW / 2, (int)DXManInstance.resH / 2);
            }

            auto currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            float currentSpeed = isShiftPressed ? cameraSpeed * 2 : cameraSpeed;
            float movementSpeed = currentSpeed * deltaTime.count();

            XMVECTOR forward = XMVectorSet(
                cos(pitch) * sin(yaw),
                sin(pitch),
                cos(pitch) * cos(yaw),
                0.0f
            );
            forward = XMVector3Normalize(forward);

            XMVECTOR right = XMVector3Normalize(XMVector3Cross(forward, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));

            if (keyWPressed) {
                cameraPosition = XMVectorAdd(cameraPosition, XMVectorScale(forward, movementSpeed));
            }
            if (keySPressed) {
                cameraPosition = XMVectorSubtract(cameraPosition, XMVectorScale(forward, movementSpeed));
            }
            if (keyAPressed) {
                cameraPosition = XMVectorAdd(cameraPosition, XMVectorScale(right, movementSpeed));
            }
            if (keyDPressed) {
                cameraPosition = XMVectorSubtract(cameraPosition, XMVectorScale(right, movementSpeed));
            }
            if (keyEPressed) {
                cameraPosition = XMVectorAdd(cameraPosition, XMVectorSet(0.0f, movementSpeed, 0.0f, 0.0f));
            }
            if (keyQPressed) {
                cameraPosition = XMVectorAdd(cameraPosition, XMVectorSet(0.0f, -movementSpeed, 0.0f, 0.0f));
            }

            XMVECTOR lookAt = XMVectorAdd(cameraPosition, forward);
            DXManInstance.viewMatrix = XMMatrixLookAtLH(cameraPosition, lookAt, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

            DXManInstance.Render();
            frameCount++;

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end - start;
            if (elapsed.count() >= 1.0) {
                LogManager::pPrint(true, true,"FPS: %d\n", frameCount);
                frameCount = 0;
                start = std::chrono::high_resolution_clock::now();
            }

        }
    }

    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_DESTROY:
        logInstance.CloseLog();
        PostQuitMessage(0);
        return 0;
    case WM_INPUT: {
        UINT dataSize;
        GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &dataSize, sizeof(RAWINPUTHEADER));
        if (dataSize > 0) {
            std::vector<BYTE> rawData(dataSize);
            if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawData.data(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize) {
                RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawData.data());
                if (raw->header.dwType == RIM_TYPEMOUSE) {
                    int dx = raw->data.mouse.lLastX;
                    int dy = raw->data.mouse.lLastY;

                    if (isMouseDown) {
                        yaw += dx * sensitivity;
                        pitch -= dy * sensitivity;

                        pitch = max(-XM_PIDIV2 + 0.01f, min(XM_PIDIV2 - 0.01f, pitch));

                        XMVECTOR forward = XMVectorSet(
                            cos(pitch) * sin(yaw),
                            sin(pitch),
                            cos(pitch) * cos(yaw),
                            0.0f
                        );
                        forward = XMVector3Normalize(forward);

                        XMVECTOR cameraPosition = XMVectorSet(0.0f, 3.0f, -40.0f, 0.0f);
                        XMVECTOR lookAt = XMVectorAdd(cameraPosition, forward);
                        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
                        DXManInstance.viewMatrix = XMMatrixLookAtLH(cameraPosition, lookAt, up);
                    }
                }
            }
        }
        return 0;
    }
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
        }
        if (wParam == 'W') {
            keyWPressed = true;
        }
        if (wParam == 'S') {
            keySPressed = true;
        }
        if (wParam == 'A') {
            keyAPressed = true;
        }
        if (wParam == 'D') {
            keyDPressed = true;
        }
        if (wParam == 'E') {
            keyEPressed = true;
        }
        if (wParam == 'Q') {
            keyQPressed = true;
        }
        if (wParam == VK_SHIFT) {
            isShiftPressed = true;
        }
        return 0;
    case WM_KEYUP:
        if (wParam == 'W') {
            keyWPressed = false;
        }
        if (wParam == 'S') {
            keySPressed = false;
        }
        if (wParam == 'A') {
            keyAPressed = false;
        }
        if (wParam == 'D') {
            keyDPressed = false;
        }
        if (wParam == 'E') {
            keyEPressed = false;
        }
        if (wParam == 'Q') {
            keyQPressed = false;
        }
        if (wParam == VK_SHIFT) {
            isShiftPressed = false;
        }
        return 0;
    case WM_RBUTTONDOWN:
        isMouseDown = true;
        ShowCursor(FALSE);
        return 0;
    case WM_RBUTTONUP:
        isMouseDown = false;
        ShowCursor(TRUE);
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
