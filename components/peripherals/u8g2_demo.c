#include "u8g2_demo.h"

static const char *TAG = "DISPLAY";

void printStrDoubleLine(u8g2_t *u8g2, char *title, char *subtitle,
                        AlignPosition position) {
  int spacingBetweenLines = 4;

  u8g2_SetFont(u8g2, u8g2_font_ncenB12_tr);
  int hTitle = u8g2_GetMaxCharHeight(u8g2);
  int wTitle = u8g2_GetStrWidth(u8g2, title);

  u8g2_SetFont(u8g2, u8g2_font_ncenB08_tr);
  int hSubTitle = u8g2_GetMaxCharHeight(u8g2);
  int wSubTitle = u8g2_GetStrWidth(u8g2, subtitle);

  int totalHeight = (hSubTitle + hTitle) + spacingBetweenLines;
  int max_w = (wTitle > wSubTitle) ? wTitle : wSubTitle;

  Point basePoint =
      get_Point(position, max_w, totalHeight, SCREEN_WIDTH, SCREEN_HEIGHT);

  u8g2_SetFont(u8g2, u8g2_font_ncenB12_tr);
  int titleX = basePoint.x + ((max_w - wTitle) / 2);
  u8g2_DrawStr(u8g2, titleX, basePoint.y + hTitle, title);

  u8g2_SetFont(u8g2, u8g2_font_ncenB08_tr);
  int subTitleX = basePoint.x + ((max_w - wSubTitle) / 2);
  u8g2_DrawStr(u8g2, subTitleX,
               basePoint.y + hSubTitle + hTitle + spacingBetweenLines,
               subtitle);
};

void drawStr(u8g2_t *u8g2, char *str, AlignPosition position) {
  int width = u8g2_GetStrWidth(u8g2, str);
  int height = u8g2_GetMaxCharHeight(u8g2);

  Point basePoint =
      get_Point(position, width, height, SCREEN_WIDTH, SCREEN_HEIGHT);

  u8g2_DrawStr(u8g2, basePoint.x, basePoint.y + height, str);
}

#define CHECKBOX_GAP 4
#define CHECKBOX_SIZE 8

void checkbox(u8g2_t *u8g2, char *str, AlignPosition position) {
  int text_width = u8g2_GetStrWidth(u8g2, str);
  int height = u8g2_GetMaxCharHeight(u8g2);

  int total_width = CHECKBOX_SIZE + CHECKBOX_GAP + text_width;

  Point basePoint =
      get_Point(position, total_width, height, SCREEN_WIDTH, SCREEN_HEIGHT);

  int box_y = basePoint.y + (height - CHECKBOX_SIZE) / 2;

  u8g2_DrawFrame(u8g2, basePoint.x, box_y, CHECKBOX_SIZE, CHECKBOX_SIZE);

  int text_x = basePoint.x + CHECKBOX_SIZE + CHECKBOX_GAP;
  u8g2_DrawStr(u8g2, text_x, basePoint.y + height, str);
}

void fillCheckBox(u8g2_t *u8g2, char *str, AlignPosition position) {
  int text_width = u8g2_GetStrWidth(u8g2, str);
  int height = u8g2_GetMaxCharHeight(u8g2);

  int total_width = CHECKBOX_SIZE + CHECKBOX_GAP + text_width;

  Point basePoint =
      get_Point(position, total_width, height, SCREEN_WIDTH, SCREEN_HEIGHT);
  int box_y = basePoint.y + (height - CHECKBOX_SIZE) / 2;

  u8g2_DrawBox(u8g2, basePoint.x, box_y, CHECKBOX_SIZE, CHECKBOX_SIZE);

  int text_x = basePoint.x + CHECKBOX_SIZE + CHECKBOX_GAP;
  u8g2_DrawStr(u8g2, text_x, basePoint.y + height, str);
}

void printIconByName(u8g2_t *u8g2, const uint8_t *icon,
                     AlignPosition position) {
  Point positionPoint =
      get_Point(position, 24, 24, SCREEN_WIDTH, SCREEN_HEIGHT);
  u8g2_DrawXBM(u8g2, positionPoint.x, positionPoint.y, 24, 24, icon);
}

void printIcon(u8g2_t *u8g2, int icon_index, AlignPosition position) {
  ESP_LOGI(TAG, "Bitmap Display %d", icon_index);
  Point positionPoint =
      get_Point(position, 24, 24, SCREEN_WIDTH, SCREEN_HEIGHT);
  u8g2_DrawXBM(u8g2, positionPoint.x, positionPoint.y, 24, 24,
               icons[icon_index]);
}

Point get_Point(AlignPosition pos, int obj_width, int obj_height,
                int screen_width, int screen_height) {
  Point point = {0, 0};

  switch (pos) {
  case TOP_LEFT:
  case CENTER_LEFT:
  case BOTTOM_LEFT:
    point.x = 0;
    break;
  case TOP_CENTER:
  case CENTER_CENTER:
  case BOTTOM_CENTER:
    point.x = (screen_width - obj_width) / 2;
    break;
  case TOP_RIGHT:
  case CENTER_RIGHT:
  case BOTTOM_RIGHT:
    point.x = screen_width - obj_width;
    break;
  }

  switch (pos) {
  case TOP_LEFT:
  case TOP_CENTER:
  case TOP_RIGHT:
    point.y = 0;
    break;
  case CENTER_LEFT:
  case CENTER_CENTER:
  case CENTER_RIGHT:
    point.y = (screen_height - obj_height) / 2;
    break;
  case BOTTOM_LEFT:
  case BOTTOM_CENTER:
  case BOTTOM_RIGHT:
    point.y = screen_height - obj_height;
    break;
  }

  return point;
}
