#include <pebble.h>

  static Window *s_main_window;
  static GFont s_custom_font_24;
  static GFont s_custom_font_18;
  static GFont s_custom_font_12;
  //static GColor bg_color;
  static TextLayer *s_time_layer;
  static TextLayer *hour_layer;
  static TextLayer *min_layer;
  static TextLayer *s_bg_layer;
  static TextLayer *am_layer;
  static TextLayer *date_layer;
  static TextLayer *temp_layer;
  static TextLayer *condition_layer;
  static TextLayer *city_layer;
  static GBitmap *no_phone_bitmap;
  static BitmapLayer *no_phone_layer;
  //static BitmapLayer *bt_layer;
  static TextLayer *s_battery_layer;
  static TextLayer *battery_text_layer;

static char date_buffer[16];


static AppSync s_sync;
static uint8_t s_sync_buffer[64];

enum WeatherKey {
  //WEATHER_ICON_KEY = 0x0,         // TUPLE_INT
  WEATHER_TEMPERATURE_KEY = 0x0,  // TUPLE_CSTRING
  WEATHER_CITY_KEY = 0x1,         // TUPLE_CSTRING
  WEATHER_CONDITION_KEY = 0X2    // TUPLE_CSTRING
};


static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    /*case WEATHER_ICON_KEY:
      if (s_icon_bitmap) {
        gbitmap_destroy(s_icon_bitmap);
      }

      s_icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[new_tuple->value->uint8]);

#ifdef PBL_SDK_3
      bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
#endif
      
      bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
      break;
*/
    case WEATHER_TEMPERATURE_KEY:
      // App Sync keeps new_tuple in s_sync_buffer, so we may use it directly
      text_layer_set_text(temp_layer, new_tuple->value->cstring);
      break;

    case WEATHER_CITY_KEY:
      text_layer_set_text(city_layer, new_tuple->value->cstring);
      break;
    case WEATHER_CONDITION_KEY:
      text_layer_set_text(condition_layer, new_tuple->value->cstring);
      break;
  }
}

static void request_weather(void) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (!iter) {
    // Error creating outbound message
    return;
  }

  int value = 1;
  dict_write_int(iter, 1, &value, sizeof(int), true);
  dict_write_end(iter);

  app_message_outbox_send();
}





static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100% chrgd";

  switch (charge_state.charge_percent) {
    case 100:
      text_layer_set_size(s_battery_layer, GSize(144, 10));
      break;
    case 90:
      text_layer_set_size(s_battery_layer, GSize(130, 10));
      break;
    case 80:
      text_layer_set_size(s_battery_layer, GSize(116, 10));
      break;
    case 70:
      text_layer_set_size(s_battery_layer, GSize(102, 10));
      break;
    case 60:
      text_layer_set_size(s_battery_layer, GSize(88, 10));
      break;
    case 50:
      text_layer_set_size(s_battery_layer, GSize(72, 10));
      break;
    case 40:
      text_layer_set_size(s_battery_layer, GSize(58, 10));
      break;
    case 30:
      text_layer_set_size(s_battery_layer, GSize(44, 10));
      break;
    case 20:
      //text_layer_set_background_color(s_battery_layer, COLOR_FALLBACK(GColorRed, GColorWhite));
      text_layer_set_size(s_battery_layer, GSize(32, 10));
      break;
    case 10:
      text_layer_set_size(s_battery_layer, GSize(18, 10));
      break;
    case 0:
      text_layer_set_size(s_battery_layer, GSize(5, 10));
      break;
  }
  
  
  
  
  if (charge_state.charge_percent > 50)
    {
    text_layer_set_text_color(battery_text_layer, GColorBlack);
  }
  else if (charge_state.charge_percent <= 20)
    {
    text_layer_set_text_color(battery_text_layer, COLOR_FALLBACK(GColorRed, GColorWhite));
    text_layer_set_background_color(s_battery_layer, COLOR_FALLBACK(GColorRed, GColorWhite));
  }
  else {
    text_layer_set_text_color(battery_text_layer, COLOR_FALLBACK(GColorWhite, GColorWhite));
    text_layer_set_background_color(s_battery_layer, COLOR_FALLBACK(GColorYellow, GColorWhite));
  }
  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text),"%d%%chrg", charge_state.charge_percent);
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(battery_text_layer, "");
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  if (tick_time->tm_min == 00) {
    vibes_short_pulse();
    light_enable_interaction();
  }
  
  if (tick_time->tm_min == 00 || tick_time->tm_min == 30)
    {
    request_weather();
  }
  
  // Create a long-lived buffer
  static char hour_buffer[] = "00";
  static char min_buffer[] = "00";
  static char am_buffer[] = "00";
  strftime(date_buffer, sizeof(date_buffer), " %a \n %b \n %e", tick_time);
  
  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(hour_buffer, sizeof("00"),  "%l", tick_time);
    strftime(min_buffer, sizeof("00"),  "%M", tick_time);
             //"%H:%M", tick_time);
    strftime(am_buffer, sizeof("00"), "%p", tick_time);
  } else {
    // Use 12 hour format
    strftime(hour_buffer, sizeof("00"),  "%l", tick_time);
    strftime(min_buffer, sizeof("00"),  "%M", tick_time);
    strftime(am_buffer, sizeof("00"), "%p", tick_time);
  }

  
  // Display this time on the TextLayer
  //text_layer_set_text(s_time_layer, buffer);
  text_layer_set_text(hour_layer, hour_buffer);
  text_layer_set_text(min_layer, min_buffer);
  text_layer_set_text(date_layer, date_buffer);
  text_layer_set_text(am_layer, am_buffer);
  
  handle_battery(battery_state_service_peek());
}



 void bt_handler(bool connected) {
  if (connected) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Phone is connected!");
      //bitmap_layer_set_bitmap(bt_layer, bt_bitmap);
      //bitmap_layer_set_background_color(bt_layer, GColorBlack);
      //bitmap_layer_destroy(no_phone_layer);
     layer_set_hidden(bitmap_layer_get_layer(no_phone_layer), true);
    layer_set_hidden(text_layer_get_layer(condition_layer), false);
    layer_set_hidden(text_layer_get_layer(temp_layer), false);
    layer_set_hidden(text_layer_get_layer(city_layer), false);
   
  request_weather();
    
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "Phone is not connected!");
    //bitmap_layer_set_bitmap(bt_layer, blank_bitmap);
    //bitmap_layer_set_background_color(bt_layer, GColorYellow);
    //layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(no_phone_layer));
    layer_set_hidden(bitmap_layer_get_layer(no_phone_layer), false);
    layer_set_hidden(text_layer_get_layer(condition_layer), true);
    layer_set_hidden(text_layer_get_layer(temp_layer), true);
    layer_set_hidden(text_layer_get_layer(city_layer), true);
    vibes_double_pulse();
    //Tuplet initial_values[] = {
    //TupletInteger(WEATHER_ICON_KEY, (uint8_t) 5),
    //TupletCString(WEATHER_TEMPERATURE_KEY, "  \u00B0F"),
    //TupletCString(WEATHER_CITY_KEY, "St Pebblesburg"),
  //};

  //app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer), 
    //  initial_values, ARRAY_LENGTH(initial_values),
      //sync_tuple_changed_callback, sync_error_callback, NULL
  //);

  //request_weather();
    update_time();
  }
}




static void main_window_load(Window *window) {
  //bg_color = GColorGreen;
  #define bg_color GColorBlack
    
    
  hour_layer = text_layer_create(GRect(-10,-15,94, 75));
  min_layer = text_layer_create(GRect(66,60,85,85));
  text_layer_set_text_color(hour_layer, COLOR_FALLBACK(GColorGreen, GColorWhite));
  text_layer_set_text_color(min_layer, COLOR_FALLBACK(GColorGreen, GColorWhite));
  
  text_layer_set_background_color(hour_layer, bg_color);
    text_layer_set_background_color(min_layer, bg_color);
  
  
  s_bg_layer = text_layer_create(GRect(0,0,144,168));
  text_layer_set_background_color(s_bg_layer, COLOR_FALLBACK(GColorBlack, GColorBlack));
  s_time_layer = text_layer_create(GRect(0,94,144,74));//0, 133, 115, 35));
  am_layer = text_layer_create(GRect(125, 150, 19, 18));
  battery_text_layer = text_layer_create(GRect(0, 150, 144, 20));
  
  date_layer = text_layer_create(GRect(70 ,0, 74, 70));
  
  s_battery_layer = text_layer_create(GRect(0, 160, 125, 20));
  
  
  condition_layer = text_layer_create(GRect(0, 70, 70, 28));  //85 px tall
  temp_layer = text_layer_create(GRect(0, 90, 70, 28));
  city_layer = text_layer_create(GRect(0, 112, 70, 28));
  
  no_phone_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NO_PHONE);
  no_phone_layer = bitmap_layer_create(GRect(0, 80, 60, 70));
  bitmap_layer_set_bitmap(no_phone_layer, no_phone_bitmap);
  bitmap_layer_set_background_color(no_phone_layer, COLOR_FALLBACK(GColorCyan, GColorWhite));
  layer_set_hidden(bitmap_layer_get_layer(no_phone_layer), true);
  bt_handler(bluetooth_connection_service_peek());

  
  ///text_layer_set_text_color(s_battery_layer, COLOR_FALLBACK(GColorWhite, GColorWhite));
  text_layer_set_background_color(s_time_layer, COLOR_FALLBACK(bg_color, GColorBlack));
  text_layer_set_background_color(am_layer, COLOR_FALLBACK(GColorClear, GColorBlack));
  text_layer_set_background_color(battery_text_layer, COLOR_FALLBACK(GColorClear, GColorClear));  //battery layer
  text_layer_set_background_color(s_battery_layer, COLOR_FALLBACK(GColorGreen, GColorWhite));
  text_layer_set_background_color(condition_layer, COLOR_FALLBACK(GColorCyan, GColorWhite));
  text_layer_set_background_color(temp_layer, COLOR_FALLBACK(GColorCyan, GColorWhite));
  text_layer_set_background_color(city_layer, COLOR_FALLBACK(GColorCyan, GColorWhite));
  text_layer_set_text_color(condition_layer, COLOR_FALLBACK(GColorBlack, GColorBlack));
  text_layer_set_text_color(temp_layer, COLOR_FALLBACK(GColorBlack, GColorBlack));
  text_layer_set_text_color(city_layer, COLOR_FALLBACK(GColorBlack, GColorBlack));
  text_layer_set_text_color(battery_text_layer, COLOR_FALLBACK(GColorOrange, GColorWhite));

  
  text_layer_set_background_color(date_layer, COLOR_FALLBACK(GColorOrange, GColorWhite));
  text_layer_set_text_color(date_layer, COLOR_FALLBACK(GColorBlack, GColorBlack));

  text_layer_set_text_color(am_layer, COLOR_FALLBACK(GColorRed, GColorWhite));

  text_layer_set_text_color(s_time_layer, COLOR_FALLBACK(GColorGreen, GColorWhite));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(battery_text_layer, GTextAlignmentCenter);
  
  text_layer_set_text_alignment(condition_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(temp_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(city_layer, GTextAlignmentCenter);
  
  
  text_layer_set_text(s_time_layer, "  :  ");
  s_custom_font_24 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CHAMPAGNE_72));
  s_custom_font_18 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CHAMPAGNE_20));//DS_DIGI_18));
  s_custom_font_12 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CHAMPAGNE_12));
  text_layer_set_font(s_time_layer, s_custom_font_24); //fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_font(date_layer, s_custom_font_18);
  text_layer_set_font(hour_layer, s_custom_font_24);
  text_layer_set_font(min_layer, s_custom_font_24);
  text_layer_set_font(condition_layer, s_custom_font_18);
  text_layer_set_font(temp_layer, s_custom_font_18);
  text_layer_set_font(city_layer, s_custom_font_12);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bg_layer));
  //text_layer_set_font(s_battery_layer, s_custom_font_18); 
  //layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
  //layer_add_child(window_get_root_layer(window), text_layer_get_layer(am_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(hour_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(min_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(condition_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(temp_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(city_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(battery_text_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(no_phone_layer));
  
  
  Tuplet initial_values[] = {
    //TupletInteger(WEATHER_ICON_KEY, (uint8_t) 1),
    TupletCString(WEATHER_TEMPERATURE_KEY, ""),
    TupletCString(WEATHER_CITY_KEY, ""),
    TupletCString(WEATHER_CONDITION_KEY, "Loading")
  };

  app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer), 
      initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL
  );

  request_weather();
}



static void main_window_unload(Window *window) {
  // Destroy TextLayer
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_bg_layer);
    text_layer_destroy(am_layer);
    text_layer_destroy(date_layer);
    text_layer_destroy(s_battery_layer);
    text_layer_destroy(hour_layer);
    text_layer_destroy(min_layer);
    text_layer_destroy(condition_layer);
    text_layer_destroy(temp_layer);
    text_layer_destroy(city_layer);
    text_layer_destroy(battery_text_layer);
    /*text_layer_destroy(s_temperature_layer);
    bitmap_layer_destroy(s_bitmap_layer);
    bitmap_layer_destroy(s_icon_layer);
    bitmap_layer_destroy(bt_layer);*/
    fonts_unload_custom_font(s_custom_font_24);
    fonts_unload_custom_font(s_custom_font_18);
    fonts_unload_custom_font(s_custom_font_12);
    //gbitmap_destroy(blank_bitmap);
    //gbitmap_destroy(bt_bitmap);
    gbitmap_destroy(no_phone_bitmap);
    bitmap_layer_destroy(no_phone_layer);
    bluetooth_connection_service_unsubscribe();
    battery_state_service_unsubscribe();
    tick_timer_service_unsubscribe();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void init() {
    // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  bluetooth_connection_service_subscribe(bt_handler);

  app_message_open(64, 64);
}




static void deinit() {
  // Destroy Window
    window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
