#include "utilities.h"

static OpMode mode;

void setMode(OpMode newMode) { mode = newMode; }
OpMode getMode() { return mode; }