#pragma once
#include "defines.h"

// Events/signals that get triggered by certain things that happen

/*
e.g. 
collider enters volume (collider)
collider exits volume (collider)

you could create a custom signal that only triggers when a collider has been inside a volume for 3 seconds
listen for Enter, start timer, if hits 3 -> emit signal. Else, reset state

Top-level triggers:
level loaded

*/