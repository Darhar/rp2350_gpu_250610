#pragma once

#include "screenManager.hpp"
#include <testscreen.h>
#include <menuscreen.h>
#include <aboutscreen.h>
#include <settingsscreen.h>
#include <basicscreen.h>
#include <splashscreen.h>

#define WAITFORSERIAL 1

void registerAllScreens(ScreenManager& mgr) {
    // Build a non-static array so lambdas can capture mgr
    //DEBUG_PRINTLN("Start");

    mgr.registerScreen(ScreenEnum::MENUSCREEN, [&mgr](){ return new MenuScreen(mgr); });
    mgr.registerScreen(ScreenEnum::TESTSCREEN, [&mgr](){ return new TestScreen(mgr); });
    mgr.registerScreen(ScreenEnum::SETTINGSSCREEN, [&mgr](){ return new SettingsScreen(mgr); });
    mgr.registerScreen(ScreenEnum::ABOUTSCREEN, [&mgr](){ return new AboutScreen(mgr); });
    mgr.registerScreen(ScreenEnum::BASICSCREEN, [&mgr](){ return new BasicScreen(mgr); });
    mgr.registerScreen(ScreenEnum::SPLASHSCREEN, [&mgr](){ return new SplashScreen(mgr); });
}
