#include <iostream>
#include <Windows.h>
#include <queue>
#include <thread>
#include <mutex>
#include <SFML/Audio.hpp>
#include <unordered_map>

std::queue<int> keypresses;
std::mutex keypress_mutex;
std::unordered_map<int, sf::SoundBuffer> keysounds;
sf::Sound KeySound;
bool playing = false;

void loadKeysounds() {
    // Load all keysounds into the unordered_map keysounds
    std::vector<int> keycodes = { VK_SPACE, VK_RETURN, VK_BACK, VK_ESCAPE, VK_LCONTROL, VK_RCONTROL, VK_LSHIFT, VK_RSHIFT };
    std::vector<std::string> filenames = { "Keys/SPACE.wav", "Keys/ENTER.wav", "Keys/BACKSPACE.wav", "Keys/ESC.wav", "Keys/CTRL.wav", "Keys/CTRL.wav", "Keys/SHIFT.wav", "Keys/SHIFT.wav" };
    for (int i = 0; i < keycodes.size(); i++) {
        sf::SoundBuffer buffer;
        if (!buffer.loadFromFile(filenames[i])) {
            std::cerr << "Error: Unable to open file " << filenames[i] << std::endl;
        }
        keysounds[keycodes[i]] = buffer;
    }

    // Load sounds for the other keys
    for (char c = 'A'; c <= 'Z'; c++) {
        std::string filename = "Keys/" + std::string(1, c) + ".wav";
        sf::SoundBuffer buffer;
        if (!buffer.loadFromFile(filename)) {
            std::cerr << "Error: Unable to open file " << filename << std::endl;
        }
        keysounds[(int)c] = buffer;
    }
}

void processKeypresses() {
    while (true) {
        keypress_mutex.lock();
        if (!keypresses.empty()) {
            int key = keypresses.front();
            keypresses.pop();
            keypress_mutex.unlock();
            if (!playing) {
                playing = true;
                KeySound.stop();
                if (keysounds.count(key) > 0) {
                    KeySound.setBuffer(keysounds[key]);
                }
                else {
                    std::string filename = "Keys/DEFAULT.wav";
                    sf::SoundBuffer buffer;
                    if (!buffer.loadFromFile(filename)) {
                        std::cerr << "Error: Unable to open file " << filename << std::endl;
                        continue;
                    }
                    keysounds[key] = buffer;
                    KeySound.setBuffer(buffer);
                }
                KeySound.play();
                while (KeySound.getStatus() == sf::Sound::Playing) {
                    // Wait until the sound has finished playing
                }
                KeySound.stop();
            }
        }
        else {
            keypress_mutex.unlock();
        }
    }
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
    if (wParam == WM_KEYDOWN) {
        keypress_mutex.lock();
        keypresses.push(pKeyboard->vkCode);
        keypress_mutex.unlock();
    }
    else if (wParam == WM_KEYUP) {
        playing = false;
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main() {
    loadKeysounds();

    std::thread processing_thread(processKeypresses);

    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (!hHook) {
        std::cerr << "Failed to install hook" << std::endl;
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook);
    return 0;
}
