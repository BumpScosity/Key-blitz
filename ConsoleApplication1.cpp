#include <iostream>
#include <Windows.h>
#include <queue>
#include <thread>
#include <mutex>
#include <SFML/Audio.hpp>

std::queue<int> keypresses;
std::mutex keypress_mutex;
sf::Music KeySound;
bool playing = false;


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
                KeySound.play();
                while (KeySound.getStatus() == sf::Music::Playing) {
                    // Wait until the music has finished playing
                }
                KeySound.setPlayingOffset(sf::seconds(0));
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

    if (!KeySound.openFromFile("Keys/Key.wav")) { // Replace Key.wav with your own sound.
        std::cerr << "Error: Unable to open file." << std::endl; // mp3 files are not supported. (Just use wav)
        return 1;
    }

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