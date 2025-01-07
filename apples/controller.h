// A class, designed to streamline reading user input
// Work in progress

#pragma once

#include <Windows.h>

class Controller {
public:
    // Worse verion of std::pair, but labeled with x and y
    template<typename T>
    class PairXY {
    public:
        T x, y;

        PairXY(T x, T y) {
            this->x = x;
            this->y = y;
        }

        PairXY<T>& operator=(const PairXY<T>& other) {
            x = other.x;
            y = other.y;
            return *this;
        }
    };

private:
    bool m_keyStates[2][256];
    bool* m_currKeyStates;
    bool* m_prevKeyStates;

    bool m_inFocus = true;

    PairXY<INT> m_windowSize = PairXY(0, 0);
    PairXY<INT> m_mousePos = PairXY(0, 0);
public:
    Controller();
    
    void processWindowMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    PairXY<INT> mousePos() const;
    PairXY<INT> windowSize() const;

    // Checks all 256 keycodes if they are down. If onlyIfInFocus is true,
    // and the window is out of focus registers all keys as up.
    void pollAllKeys(bool onlyIfInFocus = true);

    bool keyDown(UINT8 keycode) const;
    bool keyJustDown(UINT8 keycode) const;
    bool keyJustUp(UINT8 keycode) const;

};