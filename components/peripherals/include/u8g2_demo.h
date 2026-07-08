/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

/**
 * @file u8g2_demo.h
 * @brief U8G2 Demo Functions Header
 *
 * This file contains declarations for all U8G2 demo functions that showcase
 * various display capabilities including text rendering, geometric shapes,
 * pixel manipulation, progress bars, animations, and bitmap display.
 *
 * @note All demo functions expect an initialized u8g2 display object and
 *       will automatically handle buffer clearing, drawing operations, and
 *       buffer transmission to the display.
 */

#pragma once
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "icons.h"
#include "u8g2.h"
#include "u8g2_demo.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

typedef enum {
  TOP_LEFT,
  TOP_CENTER,
  TOP_RIGHT,
  CENTER_LEFT,
  CENTER_CENTER,
  CENTER_RIGHT,
  BOTTOM_LEFT,
  BOTTOM_CENTER,
  BOTTOM_RIGHT
} AlignPosition;

typedef struct {
  int x;
  int y;
} Point;

void printIcon(u8g2_t *u8g2, int icon_index, AlignPosition position);

Point get_Point(AlignPosition pos, int obj_width, int obj_height,
                int screen_width, int screen_height);

void printStrDoubleLine(u8g2_t *u8g2, char *title, char *subtitle,
                        AlignPosition position);

void printIconByName(u8g2_t *u8g2, const uint8_t *icon, AlignPosition position);

void drawStr(u8g2_t *u8g2, char *str, AlignPosition position);

void checkbox(u8g2_t *u8g2, char *str, AlignPosition position);
void fillCheckBox(u8g2_t *u8g2, char *str, AlignPosition position);

#ifdef __cplusplus
}
#endif
