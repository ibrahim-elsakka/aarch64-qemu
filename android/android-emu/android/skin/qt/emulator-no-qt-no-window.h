/* Copyright (C) 2015 The Android Open Source Project
 **
 ** This software is licensed under the terms of the GNU General Public
 ** License version 2, as published by the Free Software Foundation, and
 ** may be copied, distributed, and modified under those terms.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 */

#pragma once

#include "android/base/async/Looper.h"
#include "android/emulation/control/AdbInterface.h"
#include "android/skin/surface.h"

#include <string>
#include <vector>

// The EmulatorNoQtNoWindow class is used to build a QT-Widget-less
// event loop when the parameter -no-window is passed to the android
// emulator. Much like EmulatorQtWindow, startThread(f) will spawn
// a new thread to execute function f (qemu main loop).

#ifdef CONFIG_HEADLESS

struct QSize {
    QSize() : QSize(1, 1) { }
    QSize(int width, int height) : mWidth(width), mHeight(height) { }
    int mWidth;
    int mHeight;
    int width() const { return mWidth; }
    int height() const { return mHeight; }
    bool operator==(const QSize& other) const {
        return (mWidth == other.mWidth) &&
               (mHeight == other.mHeight);
    }
};

class SkinSurfaceBitmap;

typedef struct SkinSurface SkinSurface;

class SkinSurfaceBitmap {
    DISALLOW_COPY_AND_ASSIGN(SkinSurfaceBitmap);

public:
    SkinSurfaceBitmap(int w, int h) : mWidth(w), mHeight(h) { }
    SkinSurfaceBitmap(const char* path) : mPath(path) { }
    SkinSurfaceBitmap(const unsigned char* data, int size) : mData(size) {
        memcpy(mData.data(), data, size);
    }
    SkinSurfaceBitmap(const SkinSurfaceBitmap& other, int rotation, int blend) {
        this->mWidth = other.mWidth;
        this->mHeight = other.mHeight;
        this->mPath = other.mPath;
        this->mData = other.mData;
        mRotation = rotation;
        mBlend = blend;
        this->mData = other.mData;
    }

    ~SkinSurfaceBitmap() = default;

    QSize size() const {
        QSize res;
        res.mWidth = mWidth;
        res.mHeight = mHeight;
        return res;
    }

private:
    int mWidth = 0;
    int mHeight = 0;
    std::string mPath;
    std::vector<unsigned char> mData;
    int mRotation = 0;
    int mBlend = 0;
};

#endif

class EmulatorNoQtNoWindow final {

public:
    using Ptr = std::shared_ptr<EmulatorNoQtNoWindow>;

    static void create();
    static EmulatorNoQtNoWindow* getInstance();
    static Ptr getInstancePtr();

    void startThread(std::function<void()> looperFunction);
    void requestClose();
    void waitThread();

private:
    explicit EmulatorNoQtNoWindow();

    android::base::Looper* mLooper;
    std::unique_ptr<android::emulation::AdbInterface> mAdbInterface;
};

#ifdef CONFIG_HEADLESS

struct SkinSurface {
    int id;
    int w, h;
    int isRound;
    EmulatorNoQtNoWindow::Ptr window;
    SkinSurfaceBitmap* bitmap = nullptr;
};

#endif