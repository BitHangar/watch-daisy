/*
 * Daisy Watchface for Pebble
 * Copyright (C) 2013 Bit Hangar
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "watch_functions.h"

#define MY_UUID { 0x9D, 0xD8, 0x89, 0x5C, 0xC4, 0xA4, 0x4C, 0x4D, 0x97, 0xEC, 0x1A, 0xA6, 0xF2, 0x80, 0x5B, 0x72 }
PBL_APP_INFO(MY_UUID,
             "Daisy Clock", "Bit Hangar",
             1, 1, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;

TextLayer textLayer;
BitmapLayer hourLayer;

BmpContainer daisyContainer;
RotBmpContainer bugContainer_white;
RotBmpContainer bugContainer_black;


//Array to hold hour image resources
const int HOUR_RESOURCE_IDS[12] = {
    RESOURCE_ID_HOUR_1,
    RESOURCE_ID_HOUR_2,
    RESOURCE_ID_HOUR_3,
    RESOURCE_ID_HOUR_4,
    RESOURCE_ID_HOUR_5,
    RESOURCE_ID_HOUR_6,
    RESOURCE_ID_HOUR_7,
    RESOURCE_ID_HOUR_8,
    RESOURCE_ID_HOUR_9,
    RESOURCE_ID_HOUR_10,
    RESOURCE_ID_HOUR_11,
    RESOURCE_ID_HOUR_12
};


// Function to set the correct daisy image for the hour
void set_hour(unsigned int hour) {

  bmp_deinit_container(&daisyContainer);

  if (hour == 0)
  {
      bmp_init_container(HOUR_RESOURCE_IDS[11], &daisyContainer);
      bitmap_layer_set_bitmap(&hourLayer, &daisyContainer.bmp);
  }
  else
  {
      bmp_init_container(HOUR_RESOURCE_IDS[hour - 1], &daisyContainer);
      bitmap_layer_set_bitmap(&hourLayer, &daisyContainer.bmp);
  }
}

// Function to update the ladybug position based on the minute
void update_bug_position() {
	PblTm t;
	get_time(&t);

  set_hour(t.tm_hour % 12);

  set_hand_angle(&bugContainer_white, t.tm_min * 6);
  set_hand_angle(&bugContainer_black, t.tm_min * 6);  
}

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
    (void)ctx;
    (void)t;
    
    update_bug_position();
}

void handle_init(AppContextRef ctx) {
    (void)ctx;
    
    window_init(&window, "Daisy Clock");
    window_stack_push(&window, true /* Animated */);
    
    resource_init_current_app(&APP_RESOURCES);

    // Initialize container and bitmap layer for the hour
    bmp_init_container(RESOURCE_ID_HOUR_12, &daisyContainer);
    bitmap_layer_init(&hourLayer, window.layer.frame);
    layer_add_child(&window.layer, &hourLayer.layer);
    bitmap_layer_set_bitmap(&hourLayer, &daisyContainer.bmp);

    // Using a "hack" to get a transparent png to render without slowing the watch down to a crawl.
    // Usually a transparent png needs a RotBmpPairContainer, but in this case we are taking both
    // the white part and the black part and rendering them in separate BmpContainers, then setting
    // the compositing of each, and finally putting images on the same layer, thus creating a 
    // "transparent" bitmap. Thanks to Philip from Pebble for this "completely un-endorsed, 
    // un-official, don't-actually-do-this-in-production-code" suggestion :)
    rotbmp_init_container(RESOURCE_ID_BUG_IMAGE_WHITE, &bugContainer_white);
    bugContainer_white.layer.compositing_mode = GCompOpOr;
    rot_bitmap_set_src_ic(&bugContainer_white.layer, GPoint(14, 66));
    layer_add_child(&window.layer, &bugContainer_white.layer.layer);

    rotbmp_init_container(RESOURCE_ID_BUG_IMAGE_BLACK, &bugContainer_black);
    bugContainer_black.layer.compositing_mode = GCompOpClear;
    rot_bitmap_set_src_ic(&bugContainer_black.layer, GPoint(14, 66));
    layer_add_child(&window.layer, &bugContainer_black.layer.layer);

    update_bug_position();
}

void handle_deinit(AppContextRef ctx) {
	(void)ctx;
	
	rotbmp_deinit_container(&bugContainer_white);
  rotbmp_deinit_container(&bugContainer_black);
  bmp_deinit_container(&daisyContainer);
}

void pbl_main(void *params) {
    PebbleAppHandlers handlers = {
        .init_handler = &handle_init,
        .deinit_handler = &handle_deinit,
        
        .tick_info = {
            .tick_handler = &handle_minute_tick,
            .tick_units = MINUTE_UNIT
        }
    };
    app_event_loop(params, &handlers);
}
