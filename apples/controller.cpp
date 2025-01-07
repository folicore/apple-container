#include "controller.h"

#include <utility>
#include <Windowsx.h>

Controller::Controller() {
    for (int i = 0; i < 256; i++) {
        m_keyStates[0][i] = false;
        m_keyStates[1][i] = false;
    }

    m_currKeyStates = m_keyStates[0];
    m_prevKeyStates = m_keyStates[1];
}

void Controller::processWindowMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_SETFOCUS:
        m_inFocus = true;
    break;

    case WM_KILLFOCUS:
        m_inFocus = false;
    break;

    case WM_MOUSEMOVE: {
        m_mousePos = PairXY<INT>(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    } break;

    case WM_SIZE: {
        m_windowSize = PairXY<INT>(LOWORD(lParam), HIWORD(lParam));
    } break;
    }
}


Controller::PairXY<INT> Controller::mousePos() const {
    return m_mousePos;
}

Controller::PairXY<INT> Controller::windowSize() const {
    return m_windowSize;
}


void Controller::pollAllKeys(bool onlyIfInFocus) {
    std::swap(m_currKeyStates, m_prevKeyStates);

    for (int i = 0; i < 256; i++) {
        m_currKeyStates[i] = (m_inFocus || !onlyIfInFocus) ? (GetAsyncKeyState(i) < 0) : false;
    }
}

bool Controller::keyDown(UINT8 keycode) const {
    return m_currKeyStates[keycode];
}

bool Controller::keyJustDown(UINT8 keycode) const {
    return m_currKeyStates[keycode] && !m_prevKeyStates[keycode];
}

bool Controller::keyJustUp(UINT8 keycode) const {
    return !m_currKeyStates[keycode] && m_prevKeyStates[keycode];
}
