/**
 * @brief
 */

#pragma once
#include "defines.h"

typedef struct UI_Storage UI_Storage;

PUB bool UI_Init(UI_Storage* storage);
PUB void UI_Shutdown(UI_Storage* storage);

// TODO: define immui api
