#include "settings.h"

int get_bonus_y_position(char *p) {
    if (*p == '1')
        return BONUS_Y_BASE_POSITION;
    else if (*p == '2')
        return BONUS_Y_BASE_POSITION + 200;
    else if (*p == '3')
        return BONUS_Y_BASE_POSITION + 300;
    else if (*p == '4')
        return BONUS_Y_BASE_POSITION + 400;
    return BONUS_Y_BASE_POSITION;
}