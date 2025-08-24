#pragma once

class ScreenManager;  // fwd declare

// Register this demo’s screens
void registerAllScreens(ScreenManager& mgr);

// Demo-only helpers (temporary master side)
namespace ui_demo {
  void bind(ScreenManager& mgr);   // store pointer internally
  void setup_master();             // config I2C master (dev only)
  void run_master();               // optional dev traffic
  void core1_entry();          // <-- add this declaration
}
