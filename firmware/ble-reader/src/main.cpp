#include <Arduino.h>

#include "reader_app.h"

namespace {
pocket::ReaderApp app;
}

void setup() { app.begin(); }

void loop() { app.loop(); }
