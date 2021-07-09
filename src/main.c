#include <pebble.h>

//#define DEMO_MODE
#define DEMO_TIME_24H "10:45"
#define DEMO_TIME_12H "08:30 PM"
#define DEMO_DATE "Saturday 31"
#define DEMO_VERSE_REF "John 10:10" //"Psalm 1:1"
#define DEMO_VERSE_TEXT "The thief comes only to steal and kill and destroy; I have come that they may have life, and have it to the full." //"How well God must like you— you don’t hang out at Sin Saloon, you don’t slink along Dead-End Road, you don’t go to Smart-Mouth College."

#define PERSISTENT_STORAGE_VERSION 1 // increment when storage values change

#define KEY_VERSE_REFERENCE 0
#define KEY_VERSE_TEXT 1
#define KEY_DATE_FORMAT 2
#define KEY_UPDATE_TIME 3
#define KEY_ENABLE_LIGHT 4
#define KEY_BT_VIBE 5
#define KEY_INVERT_COLOURS 6
#define KEY_SHOW_AMPM 7
#define KEY_VERSE_FONT 8
#define KEY_SCROLL_SPEED 9
#define KEY_REQUEST_SETTINGS 10

#define PERSIST_KEY_DATEFORMAT 0
#define PERSIST_KEY_STORAGEVERSION 1
#define PERSIST_KEY_UPDATETIME 2
#define PERSIST_KEY_ENABLELIGHT 3
#define PERSIST_KEY_BTVIBE 4
#define PERSIST_KEY_INVERTCOLOURS 5
#define PERSIST_KEY_SHOWAMPM 6
#define PERSIST_KEY_VERSEFONT 7
#define PERSIST_KEY_SCROLLSPEED 8

#define DEFAULT_DATE_FORMAT 0
#define DEFAULT_UPDATE_TIME 360 // 6am
#define DEFAULT_ENABLE_LIGHT true
#define DEFAULT_BT_VIBE true
#define DEFAULT_INVERT_COLOURS false
#define DEFAULT_SHOW_AMPM true
#define DEFAULT_VERSE_FONT VERSE_FONT_MEDIUM
#define DEFAULT_SCROLL_SPEED SCROLL_SPEED_MEDIUM

#define DEFAULT_DATE_STRING "DAY ## MTH"
#define DEFAULT_TIME_STRING "HH:MMxx"
#define VERSE_ANIMATION_DURATION 400
#define VERSE_WAIT_TIME_SCROLL 6000 // TODO: s_settingScrollSpeed should affect wait times!
#define VERSE_WAIT_TIME_NOSCROLL 15000
//#define MINUTES_PER_DAY 1440
#define VERSE_TEXT_VISIBLE_HEIGHT 114
#define UPDATE_INTERVAL 20000
#define REFERENCE_MAX_SIZE 25
#define VERSE_MAX_SIZE 1000
#define MAX_VERSE_REQUESTS_PER_HOUR 10
#define BATTERY_ON 0
#define BATTERY_CHARGING 1
#define BATTERY_PLUGGED 2
#define SHAKE_AWAKE_TIMEOUT 3000
#define LIGHT_CHECK_TIMEOUT 3000
#define HIDE_STATUS_TIMEOUT 5000
#define VERSE_BOX_MAX_HEIGHT 500

#define SCROLL_SPEED_PX_PER_MS_SLOW 0.008f
#define WAIT_TIME_SCROLL_MS_SLOW 6000
#define WAIT_TIME_NO_SCROLL_MS_SLOW 15000
#define SCROLL_SPEED_PX_PER_MS_MEDIUM 0.014f
#define WAIT_TIME_SCROLL_MS_MEDIUM 4000
#define WAIT_TIME_NO_SCROLL_MS_MEDIUM 12000
#define SCROLL_SPEED_PX_PER_MS_FAST 0.02f
#define WAIT_TIME_SCROLL_MS_FAST 2000
#define WAIT_TIME_NO_SCROLL_MS_FAST 9000

#define GRECT_BLUETOOTH_OUT GRect(-20, 90, 20, 16)
#define GRECT_BLUETOOTH_IN GRect(58, 90, 20, 16)
#define GRECT_BLUETOOTH_OFFSET GRect(26, 90, 20, 16)
#define GRECT_BATTERY_OUT GRect(168, 90, 42, 16)
#define GRECT_BATTERY_IN GRect(49, 90, 42, 16)
#define GRECT_BATTERY_OFFSET GRect(76, 90, 42, 16)

#define FONT_TIME RESOURCE_ID_FONT_TIME_96
#define FONT_AMPM RESOURCE_ID_FONT_AMPM_38

#define NUM_INVERTER_LAYERS_PER_BORDER 6

#define VERSE_TEXT_Y_COORD 27
	
#define VERSE_TEXT_PADDING_SMALL 4
#define VERSE_TEXT_PADDING_MEDIUM 7
#define VERSE_TEXT_PADDING_LARGE 10
#define VERSE_TEXT_PADDING_XLARGE 10

enum {
	VERSE_FONT_SMALL = 0,
	VERSE_FONT_MEDIUM = 1,
	VERSE_FONT_LARGE = 2,
	VERSE_FONT_XLARGE = 3,
} VerseFont_t;

enum {
	SCROLL_SPEED_SLOW = 0,
	SCROLL_SPEED_MEDIUM = 1,
	SCROLL_SPEED_FAST = 2,
} ScrollSpeed_t;

typedef struct {
	GFont font;
	int16_t padding;
} VerseTextSettings_t;

typedef struct {
	uint16_t waitTimeScroll;
	uint16_t waitTimeNoScroll;
	float scrollSpeed;
} VerseScrollSettings_t;

static Window* s_main_window;
static TextLayer* s_layer_time;
static TextLayer* s_layer_date;
static TextLayer* s_layer_verseRef;
static TextLayer* s_layer_amPm;
static Layer* s_layer_timeDisplay;
static TextLayer* s_layer_verseText;
static BitmapLayer* s_layer_borderTop;
static BitmapLayer* s_layer_borderBottom;
static BitmapLayer* s_layer_borderLeft;
static BitmapLayer* s_layer_borderRight;
static TextLayer* s_layer_miniTime;
static BitmapLayer* s_layer_bluetoothLogo;
static BitmapLayer* s_layer_bluetoothStatus;
static BitmapLayer* s_layer_batteryIcon;
static BitmapLayer* s_layer_batteryColon;
static TextLayer* s_layer_batteryLevel;
static Layer* s_layer_bluetoothGroup;
static Layer* s_layer_batteryGroup;
static InverterLayer* s_layer_inverterMain; // used as flag - if != NULL, colours are inverted
static InverterLayer* s_layers_inverterTop[NUM_INVERTER_LAYERS_PER_BORDER];
static InverterLayer* s_layers_inverterBottom[NUM_INVERTER_LAYERS_PER_BORDER];

static GBitmap* s_bitmap_bordersVert;
static GBitmap* s_bitmap_bordersHoriz;
static GBitmap* s_bitmap_borderTop;
static GBitmap* s_bitmap_borderBottom;
static GBitmap* s_bitmap_borderLeft;
static GBitmap* s_bitmap_borderRight;
static GBitmap* s_bitmap_statusBarIcons;
static GBitmap* s_bitmap_bluetoothLogo;
static GBitmap* s_bitmap_bluetoothStatusConnected;
static GBitmap* s_bitmap_bluetoothStatusDisconnected;
static GBitmap* s_bitmap_batteryIconBattery;
static GBitmap* s_bitmap_batteryIconCharging;
static GBitmap* s_bitmap_batteryIconPlugged;
static GBitmap* s_bitmap_batteryColon;

static PropertyAnimation* s_animation_miniTime;
static PropertyAnimation* s_animation_borderTopCompress;
static PropertyAnimation* s_animation_borderTopExpand;
static PropertyAnimation* s_animation_borderBottomCompress;
static PropertyAnimation* s_animation_borderBottomExpand;
static PropertyAnimation* s_animation_verseScroll;
static PropertyAnimation* s_animation_bluetooth;
static PropertyAnimation* s_animation_battery;

static AppTimer* s_timer_verseDisplayTimer;
static AppTimer* s_timer_requestVerseTimer;
static AppTimer* s_timer_shakeAwake;
static AppTimer* s_timer_light;
static AppTimer* s_timer_hideStatus;

static bool s_gotVerse;
static bool s_verseShown;
static uint8_t s_verseRequests;
static bool s_needScroll;
static bool s_bluetoothConnected;
static uint8_t s_batteryStatus;
static uint8_t s_batteryLevel;
static bool s_shakeAwake;
static uint8_t s_verseTextYCoord;
static VerseScrollSettings_t s_verseScrollSettings;
static bool s_requestSettings;

// settable through settings
static uint16_t s_settingDateFormat = DEFAULT_DATE_FORMAT;
static uint16_t s_settingUpdateTime = DEFAULT_UPDATE_TIME;
static uint16_t s_settingEnableLight = DEFAULT_ENABLE_LIGHT;
static bool s_settingBtVibe = DEFAULT_BT_VIBE;
static bool s_settingInvertColours = DEFAULT_INVERT_COLOURS;
static bool s_settingShowAmPm = DEFAULT_SHOW_AMPM;
static uint16_t s_settingVerseFont = DEFAULT_VERSE_FONT;
static uint16_t s_settingScrollSpeed = DEFAULT_SCROLL_SPEED;

static GFont s_font_time;
static GFont s_font_amPm;

char* translate_error(AppMessageResult result)
{
  switch (result) {
    case APP_MSG_OK: return "OK";
    case APP_MSG_SEND_TIMEOUT: return "SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "INVALID_ARGS";
    case APP_MSG_BUSY: return "BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}

static void cancelTimer(AppTimer* timer)
{
	if (timer != NULL)
	{
		app_timer_cancel(timer);
		timer = NULL;
	}
}

static void adjustVerseTextPosition(void)
{
	GSize contentSize = text_layer_get_content_size(s_layer_verseText);
	GRect origFrame = layer_get_frame((Layer*)s_layer_verseText);
	
	uint8_t yCoord = s_verseTextYCoord;
	
	if (contentSize.h <= VERSE_TEXT_VISIBLE_HEIGHT)
		yCoord += ((VERSE_TEXT_VISIBLE_HEIGHT - contentSize.h) >> 1);
	
	layer_set_frame(text_layer_get_layer(s_layer_verseText), GRect(origFrame.origin.x, yCoord, origFrame.size.w, origFrame.size.h));
}

static void checkLight(void* data)
{
	if (s_verseShown && s_settingEnableLight)
	{
		light_enable_interaction();
		s_timer_light = app_timer_register(LIGHT_CHECK_TIMEOUT, checkLight, NULL);
	}
		
	else
		cancelTimer(s_timer_light);		
}

static void borderTopCompressedCB(Animation* animation, bool finished, void* context)
{
	animation_destroy(animation);
	
	if (s_verseShown)
		adjustVerseTextPosition();
	
	layer_set_hidden(s_layer_timeDisplay, s_verseShown);
	layer_set_hidden((Layer*)s_layer_verseText, !s_verseShown);
	
	if (!s_verseShown)
		animation_schedule((Animation*)s_animation_miniTime);
	
	animation_schedule((Animation*)s_animation_borderTopExpand);
	
	checkLight(NULL);
}

static void borderBottomCompressedCB(Animation* animation, bool finished, void* context)
{
	animation_schedule((Animation*)s_animation_borderBottomExpand);
}

static void scheduleScrollAnimation(void* data)
{
	animation_schedule((Animation*)s_animation_verseScroll);
	s_timer_verseDisplayTimer = NULL;
}

static void toggleVerseDisplay(void)
{	
	if(s_verseShown)
	{
		if (s_animation_verseScroll != NULL)
			animation_unschedule((Animation*)s_animation_verseScroll);
		
		cancelTimer(s_timer_verseDisplayTimer);
	}
	
	s_verseShown = !s_verseShown;
	
	animation_schedule((Animation*)s_animation_borderTopCompress);	
	animation_schedule((Animation*)s_animation_borderBottomCompress);
	if (s_verseShown)
		animation_schedule((Animation*)s_animation_miniTime);
}

static void timerCbHideVerse(void* data)
{
	toggleVerseDisplay();
	s_timer_verseDisplayTimer = NULL;
}

static void startVerseDisplayTimer(void)
{
	cancelTimer(s_timer_verseDisplayTimer);
	
	if (s_needScroll)
		s_timer_verseDisplayTimer = app_timer_register(s_verseScrollSettings. waitTimeScroll, scheduleScrollAnimation, NULL);
	else
		s_timer_verseDisplayTimer = app_timer_register(s_verseScrollSettings. waitTimeNoScroll, timerCbHideVerse, NULL);	
}

static void verseScrollFinishedCallback(Animation* animation, bool finished, void* context)
{	
	if (finished)
	{
		cancelTimer(s_timer_verseDisplayTimer);
		s_timer_verseDisplayTimer = app_timer_register(s_verseScrollSettings. waitTimeScroll, timerCbHideVerse, NULL);
	}
}

static void borderTopExpandedCB(Animation* animation, bool finished, void* context);

static void initBorderTopAnimations(void)
{
	GRect borderTopCompressed = GRect(0, -8, 144, 84);
	GRect borderTopVerse = GRect(0, -57, 144, 84);
	GRect borderTopExpanded = GRect(0, -73, 144, 84);
	GRect borderTopFrom, borderTopTo;
	
	if (!s_verseShown)
	{
		borderTopFrom = borderTopExpanded;
		borderTopTo = borderTopVerse;
	}
	else
	{
		borderTopFrom = borderTopVerse;
		borderTopTo = borderTopExpanded;
	}

	s_animation_borderTopCompress = property_animation_create_layer_frame((Layer*)s_layer_borderTop, &borderTopFrom, &borderTopCompressed);
	s_animation_borderTopExpand = property_animation_create_layer_frame((Layer*)s_layer_borderTop, &borderTopCompressed, &borderTopTo);
	
	animation_set_curve((Animation*)s_animation_borderTopCompress, AnimationCurveEaseIn);
	animation_set_curve((Animation*)s_animation_borderTopExpand, AnimationCurveEaseOut);
	
	animation_set_duration((Animation*) s_animation_borderTopCompress, VERSE_ANIMATION_DURATION);
	animation_set_duration((Animation*) s_animation_borderTopExpand, VERSE_ANIMATION_DURATION);

	animation_set_handlers( (Animation*)s_animation_borderTopCompress,
		(AnimationHandlers) { .stopped = (AnimationStoppedHandler)borderTopCompressedCB },
		NULL );		
	animation_set_handlers( (Animation*)s_animation_borderTopExpand,
		(AnimationHandlers) { .stopped = (AnimationStoppedHandler)borderTopExpandedCB },
		NULL );	
}

void borderTopExpandedCB(Animation* animation, bool finished, void* context)
{
	animation_destroy(animation);
	
	initBorderTopAnimations();
	
	if (s_verseShown)
		startVerseDisplayTimer();
}

static void initBorderBottomAnimations(void)
{
	Layer* borderBottomLayer = bitmap_layer_get_layer(s_layer_borderBottom);
	GRect borderBottomCompressed = GRect(0, 76, 144, 84);
	GRect borderBottomExpanded = GRect(0, 141, 144, 84);
	
	s_animation_borderBottomCompress = property_animation_create_layer_frame(borderBottomLayer, &borderBottomExpanded, &borderBottomCompressed);
	s_animation_borderBottomExpand = property_animation_create_layer_frame(borderBottomLayer, &borderBottomCompressed, &borderBottomExpanded);
	
	animation_set_curve((Animation*)s_animation_borderBottomCompress, AnimationCurveEaseIn);
	animation_set_curve((Animation*)s_animation_borderBottomExpand, AnimationCurveEaseOut);
	animation_set_duration((Animation*) s_animation_borderBottomCompress, VERSE_ANIMATION_DURATION);
	animation_set_duration((Animation*) s_animation_borderBottomExpand, VERSE_ANIMATION_DURATION);
	
	animation_set_handlers( (Animation*)s_animation_borderBottomCompress,
		(AnimationHandlers) { .stopped = (AnimationStoppedHandler)borderBottomCompressedCB },
		NULL );
}

static void miniTimeAnimationCB(Animation* animation, bool finished, void* context);

static void initMiniTimeAnimation(void)
{
	GRect miniTimeShown = GRect(40, -1, 64, 16);
	GRect miniTimeHidden = GRect(40, -16, 64, 16);
	
	if (!s_verseShown)
		s_animation_miniTime = property_animation_create_layer_frame((Layer*)s_layer_miniTime, &miniTimeHidden, &miniTimeShown);
	else
		s_animation_miniTime = property_animation_create_layer_frame((Layer*)s_layer_miniTime, &miniTimeShown, &miniTimeHidden);
	
	animation_set_duration((Animation*) s_animation_miniTime, VERSE_ANIMATION_DURATION);
	
	animation_set_handlers( (Animation*)s_animation_miniTime,
		(AnimationHandlers) { .stopped = (AnimationStoppedHandler)miniTimeAnimationCB },
		NULL );
}

void miniTimeAnimationCB(Animation* animation, bool finished, void* context)
{
	animation_destroy(animation);
	initMiniTimeAnimation();
}

static void hideStatus(void);

static void shakeAwakeTimeoutCB(void* data)
{
	s_shakeAwake = false;
	
	hideStatus();
	
	s_timer_shakeAwake = NULL;
}

static bool animationRunning(void)
{
	if ( animation_is_scheduled((Animation*)s_animation_miniTime) ||
		 animation_is_scheduled((Animation*)s_animation_borderTopCompress) ||
		 animation_is_scheduled((Animation*)s_animation_borderTopExpand) ||
		 animation_is_scheduled((Animation*)s_animation_borderBottomCompress) ||
		 animation_is_scheduled((Animation*)s_animation_borderBottomExpand) )
		return true;
	
	return false;
}

static void showStatus(void);

static void tap_handler(AccelAxisType axis, int32_t direction)
{	
	if (animationRunning())
		return;
	
	// TODO: use accelerometer to toggle
	if (s_verseShown || s_shakeAwake)
		toggleVerseDisplay();
	else if (!s_shakeAwake)
	{
		s_shakeAwake = true;
		showStatus();

		// start timer to hide status and unset shakeAwake
		s_timer_shakeAwake = app_timer_register(SHAKE_AWAKE_TIMEOUT, shakeAwakeTimeoutCB, NULL);
	}
}

static void sendAppMessage(void)
{
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	
	if (iter)
	{
		if (s_requestSettings)
			dict_write_uint8(iter, KEY_REQUEST_SETTINGS, 0x01);
		
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Sending from Pebble: requestSettings(%d)", s_requestSettings);
		app_message_outbox_send();
	}
}

static void requestVerseTimerCallback(void* data)
{
	if (s_verseRequests < MAX_VERSE_REQUESTS_PER_HOUR)
	{
		sendAppMessage();
		s_verseRequests += 1;
	}
	else
	{
		s_timer_requestVerseTimer = NULL;

		//if (!s_gotVerse)
		//	text_layer_set_text(s_layer_verseText, ":-(\n\nCouldn't get verse!\nPlease check data connection and restart watchface");
	}
}

static void getVerse(void)
{
	s_gotVerse = false;
	s_verseRequests = 0;
	
	cancelTimer(s_timer_requestVerseTimer);
	
	sendAppMessage();
}

static void update_time(void)
{
	static char timeBuf[] = DEMO_TIME_24H;
	
	time_t temp = time(NULL); 
	struct tm *tick_time = localtime(&temp);

	if(clock_is_24h_style() == true)
	{
#ifndef DEMO_MODE
		strftime(timeBuf, sizeof(timeBuf), "%H:%M", tick_time);
#endif
		text_layer_set_text(s_layer_miniTime, timeBuf);
		text_layer_set_text(s_layer_time, timeBuf);
	}
	else
	{
		static char timeBuf12h[] = DEMO_TIME_12H;
#ifndef DEMO_MODE
		strftime(timeBuf12h, sizeof(timeBuf12h), "%I:%M %p", tick_time);
#endif
		
		char* pBufStart = timeBuf12h;
		
		if (timeBuf12h[0] == '0')
			pBufStart = &timeBuf12h[1];
		
		text_layer_set_text(s_layer_miniTime, pBufStart);
		
		uint8_t i;		
		for (i = 0; (pBufStart < &timeBuf12h[5]); i++, pBufStart++)
			timeBuf[i] = *pBufStart;
		
		timeBuf[i] = '\0'; // time without AM/PM
		text_layer_set_text(s_layer_time, timeBuf);
		
		if (s_settingShowAmPm)
		{
			if (s_layer_amPm == NULL)
			{
				s_font_amPm = fonts_load_custom_font(resource_get_handle(FONT_AMPM));

				// create layer, re-posistion time layer, add to timeDisplay layer
				s_layer_amPm = text_layer_create(GRect(110, 0, 24, 40));
				text_layer_set_background_color(s_layer_amPm, GColorBlack);
				text_layer_set_text_color(s_layer_amPm, GColorWhite);
				text_layer_set_text(s_layer_amPm, "pp");
				text_layer_set_font(s_layer_amPm, s_font_amPm);
				text_layer_set_text_alignment(s_layer_amPm, GTextAlignmentLeft);

				layer_add_child(s_layer_timeDisplay, (Layer*)s_layer_amPm);
				text_layer_set_text_alignment(s_layer_time, GTextAlignmentLeft);
			}

			text_layer_set_text(s_layer_amPm, &timeBuf12h[6]); // "AM" or "PM"

			uint8_t amPmYCoord = 48;

			GSize timeSize = text_layer_get_content_size(s_layer_time);

			int16_t timeXCoord = (112 - timeSize.w) >> 1; // centralise with space for amPm
			layer_set_frame((Layer*)s_layer_time, GRect(timeXCoord, -10, (136 - timeXCoord), 100));

			layer_set_frame((Layer*)s_layer_amPm, GRect((timeXCoord + timeSize.w), amPmYCoord, 24, 40));
		}
		
		else if (s_layer_amPm != NULL)
		{
			text_layer_destroy(s_layer_amPm);
			s_layer_amPm = NULL;
			text_layer_set_text_alignment(s_layer_time, GTextAlignmentCenter);
			layer_set_frame((Layer*)s_layer_time, GRect(0, -10, 136, 100)); // TODO: macro default time frame
		}
	}
		
	uint16_t minsSinceMidnight = (tick_time->tm_hour * 60) + tick_time->tm_min;
	
	// TODO: more robust method to make sure we always have the new verse ASAP after s_settingUpdateTime -- consider phone being off at night etc
	if ( (minsSinceMidnight == s_settingUpdateTime) || !s_gotVerse) //(!s_gotVerse && !s_gotVerse(tick_time->tm_min == 0)) )
		getVerse();
}

static void update_date(void)
{
	time_t temp = time(NULL); 
	struct tm *tick_time = localtime(&temp);

	static char date_buffer[25] = DEFAULT_DATE_STRING;
	char date_format_string[9];

	switch(s_settingDateFormat)
	{
	case 0: // Sat 31 Jan
		memcpy(date_format_string, "%a %e %b", sizeof(date_format_string));
		break;
	case 1: // Sat Jan 31
		memcpy(date_format_string, "%a %b %e", sizeof(date_format_string));
		break;
	case 2: // 31 Jan
		memcpy(date_format_string, "%e %b", sizeof(date_format_string));
		break;
	case 3: // Jan 31
		memcpy(date_format_string, "%b %e", sizeof(date_format_string));
		break;
	case 4: // Sat 31
		memcpy(date_format_string, "%a %e", sizeof(date_format_string));
		break;
	/* case 5 is default */
	case 6: // dd/mm/yyyy
		memcpy(date_format_string, "%d/%m/%Y", sizeof(date_format_string));
		break;
	case 7: // mm/dd/yyyy
		memcpy(date_format_string, "%m/%d/%Y", sizeof(date_format_string));
		break;
	case 8: // dd/mm/yy
		memcpy(date_format_string, "%d/%m/%y", sizeof(date_format_string));
		break;
	case 9: // mm/dd/yy
		memcpy(date_format_string, "%m/%d/%y", sizeof(date_format_string));
		break;
	default: // Saturday 31
		memcpy(date_format_string, "%A %e", sizeof(date_format_string));
		break;
	}
	
	strftime(date_buffer, sizeof(date_buffer), date_format_string, tick_time);
		
#ifdef DEMO_MODE
	strcpy(date_buffer, DEMO_DATE);
#endif

	text_layer_set_text(s_layer_date, date_buffer);
}

static void tick_handler(struct tm* tick_time, TimeUnits units_changed)
{	
	if ((units_changed & MINUTE_UNIT) == MINUTE_UNIT)
		update_time();
	
	if ((units_changed & DAY_UNIT) == DAY_UNIT)
		update_date();
}

static char* formatBatteryLevel(void)
{
	static char batteryLevelBuf[5];

	uint8_t strSize = snprintf(batteryLevelBuf, sizeof(batteryLevelBuf), "%d", s_batteryLevel);
	batteryLevelBuf[strSize] = '\%';
	batteryLevelBuf[strSize+1] = '\0';
	
	return batteryLevelBuf;
}

void animationDestroyCB(Animation* animation, bool finished, void* context)
{
	animation_destroy(animation);
	
	if (animation == (Animation*)s_animation_bluetooth)
		s_animation_bluetooth = NULL;
	else if (animation == (Animation*)s_animation_battery)
		s_animation_battery = NULL;
}

static void setupBluetoothAnimation(GRect* pFrameTo, AnimationCurve curve)
{
	GRect frameFrom = layer_get_frame(s_layer_bluetoothGroup);
	
	s_animation_bluetooth = property_animation_create_layer_frame(s_layer_bluetoothGroup, &frameFrom, pFrameTo);
	animation_set_duration((Animation*) s_animation_bluetooth, VERSE_ANIMATION_DURATION);
	animation_set_curve((Animation*)s_animation_bluetooth, curve);
	
	animation_set_handlers( (Animation*)s_animation_bluetooth,
		(AnimationHandlers) { .stopped = (AnimationStoppedHandler)animationDestroyCB },
		NULL );
}

static void setupBatteryAnimation(GRect* pFrameTo, AnimationCurve curve)
{
	GRect frameFrom = layer_get_frame(s_layer_batteryGroup);
	
	s_animation_battery = property_animation_create_layer_frame(s_layer_batteryGroup, &frameFrom, pFrameTo);
	animation_set_duration((Animation*) s_animation_battery, VERSE_ANIMATION_DURATION);
	animation_set_curve((Animation*)s_animation_battery, curve);
	
	animation_set_handlers( (Animation*)s_animation_battery,
		(AnimationHandlers) { .stopped = (AnimationStoppedHandler)animationDestroyCB },
		NULL );
}

void showStatus(void)
{
	// TODO: make static const
	GRect bluetoothFrameTo = GRECT_BLUETOOTH_OFFSET;
	GRect batteryFrameTo = GRECT_BATTERY_OFFSET;
	
	setupBluetoothAnimation(&bluetoothFrameTo, AnimationCurveEaseOut);
	setupBatteryAnimation(&batteryFrameTo, AnimationCurveEaseOut);
	
	animation_schedule((Animation*)s_animation_bluetooth);
	animation_schedule((Animation*)s_animation_battery);
}

void hideStatus(void)
{
	GRect bluetoothFrameTo, batteryFrameTo;
	
	bool hideBoth = s_bluetoothConnected && (s_batteryStatus == BATTERY_ON);
	bool hideBattery = !s_bluetoothConnected && (s_batteryStatus == BATTERY_ON);
	bool hideBluetooth = s_bluetoothConnected && (s_batteryStatus != BATTERY_ON);

	// TODO: refactor
	if (hideBoth || hideBluetooth || hideBattery)
	{
		if (hideBoth || hideBluetooth)
		{
			bluetoothFrameTo = GRECT_BLUETOOTH_OUT;
			setupBluetoothAnimation(&bluetoothFrameTo, AnimationCurveEaseIn);
		}
		else if (hideBattery)
		{
			bluetoothFrameTo = GRECT_BLUETOOTH_IN;
			setupBluetoothAnimation(&bluetoothFrameTo, AnimationCurveEaseInOut);
		}

		if (hideBoth || hideBattery)
		{
			batteryFrameTo = GRECT_BATTERY_OUT;
			setupBatteryAnimation(&batteryFrameTo, AnimationCurveEaseIn);
		}
		else if (hideBluetooth)
		{
			batteryFrameTo = GRECT_BATTERY_IN;
			setupBatteryAnimation(&batteryFrameTo, AnimationCurveEaseInOut);
		}
		
		animation_schedule((Animation*)s_animation_bluetooth);
		animation_schedule((Animation*)s_animation_battery);	
	}
}

void showStatusSticky(void)
{
	GRect bluetoothFrameTo, batteryFrameTo;
	
	bool showBoth = !s_bluetoothConnected && (s_batteryStatus != BATTERY_ON);
	bool showBattery = s_bluetoothConnected && (s_batteryStatus != BATTERY_ON);
	bool showBluetooth = !s_bluetoothConnected && (s_batteryStatus == BATTERY_ON);

	// TODO: refactor
	if (showBoth)
		showStatus();
	else
	{
		if (showBattery)
		{
			batteryFrameTo = GRECT_BATTERY_IN;
			bluetoothFrameTo = GRECT_BLUETOOTH_OUT;
			setupBatteryAnimation(&batteryFrameTo, AnimationCurveEaseOut);
			setupBluetoothAnimation(&bluetoothFrameTo, AnimationCurveEaseIn);
		}
		else if (showBluetooth)
		{
			batteryFrameTo = GRECT_BATTERY_OUT;
			bluetoothFrameTo = GRECT_BLUETOOTH_IN;
			setupBatteryAnimation(&batteryFrameTo, AnimationCurveEaseIn);
			setupBluetoothAnimation(&bluetoothFrameTo, AnimationCurveEaseOut);			
		}
		
		animation_schedule((Animation*)s_animation_bluetooth);
		animation_schedule((Animation*)s_animation_battery);	
	}
}

static void hideStatusTimerCB(void* data)
{
	hideStatus();
}

static void startHideStatusTimer(void)
{
	cancelTimer(s_timer_hideStatus);
	
	s_timer_hideStatus = app_timer_register(HIDE_STATUS_TIMEOUT, hideStatusTimerCB, NULL);
}

static void updateBluetoothStatus(bool connected)
{
	s_bluetoothConnected = connected;
	
	if (s_bluetoothConnected)
		bitmap_layer_set_bitmap(s_layer_bluetoothStatus, s_bitmap_bluetoothStatusConnected);
	else
		bitmap_layer_set_bitmap(s_layer_bluetoothStatus, s_bitmap_bluetoothStatusDisconnected);
}

static void bluetoothConnectionHandler(bool connected)
{	
	VibePattern vibePattern;
	
	if (connected)
	{
		static const uint32_t btConnPattern[] = { 100, 300, 100 };
		vibePattern = (VibePattern) {
			.durations = btConnPattern,
			.num_segments = ARRAY_LENGTH(btConnPattern)
		};
	}
	else
	{
		static const uint32_t btDisconPattern[] = { 100, 300, 400 };
		vibePattern = (VibePattern) {
			.durations = btDisconPattern,
			.num_segments = ARRAY_LENGTH(btDisconPattern)
		};
	}
	
	updateBluetoothStatus(connected);
	
	if (s_settingBtVibe)
		vibes_enqueue_custom_pattern(vibePattern);
	
	if (!s_bluetoothConnected)
		showStatusSticky();
	else
		startHideStatusTimer();
}

static void updateBatteryState(BatteryChargeState batteryState)
{
	s_batteryLevel = batteryState.charge_percent;
	
	if (batteryState.is_charging)
	{
		s_batteryStatus = BATTERY_CHARGING;
		bitmap_layer_set_bitmap(s_layer_batteryIcon, s_bitmap_batteryIconCharging);
	}
	else if (batteryState.is_plugged)
	{
		s_batteryStatus = BATTERY_PLUGGED;
		bitmap_layer_set_bitmap(s_layer_batteryIcon, s_bitmap_batteryIconPlugged);
	}
	else
	{
		s_batteryStatus = BATTERY_ON;
		bitmap_layer_set_bitmap(s_layer_batteryIcon, s_bitmap_batteryIconBattery);
	}
	
	text_layer_set_text(s_layer_batteryLevel, formatBatteryLevel());
}

static void batteryStateHandler(BatteryChargeState batteryState)
{
	s_batteryLevel = batteryState.charge_percent;
	
	if (batteryState.is_charging)
		s_batteryStatus = BATTERY_CHARGING;
	else if (batteryState.is_plugged)
		s_batteryStatus = BATTERY_PLUGGED;
	else
		s_batteryStatus = BATTERY_ON;
	
	updateBatteryState(batteryState);
	
	if (s_batteryStatus != BATTERY_ON)
		showStatusSticky();
	else
		startHideStatusTimer();
}

static void setupVerseScrollSettings(void)
{
	switch(s_settingScrollSpeed)
	{
	case SCROLL_SPEED_SLOW:
		s_verseScrollSettings.scrollSpeed = SCROLL_SPEED_PX_PER_MS_SLOW;
		s_verseScrollSettings.waitTimeScroll = WAIT_TIME_SCROLL_MS_SLOW;
		s_verseScrollSettings.waitTimeNoScroll = WAIT_TIME_NO_SCROLL_MS_SLOW;
		break;
	case SCROLL_SPEED_FAST:
		s_verseScrollSettings.scrollSpeed = SCROLL_SPEED_PX_PER_MS_FAST;
		s_verseScrollSettings.waitTimeScroll = WAIT_TIME_SCROLL_MS_FAST;
		s_verseScrollSettings.waitTimeNoScroll = WAIT_TIME_NO_SCROLL_MS_FAST;
		break;
	default: // SCROLL_SPEED_MEDIUM is default
		s_verseScrollSettings.scrollSpeed = SCROLL_SPEED_PX_PER_MS_MEDIUM;
		s_verseScrollSettings.waitTimeScroll = WAIT_TIME_SCROLL_MS_MEDIUM;
		s_verseScrollSettings.waitTimeNoScroll = WAIT_TIME_NO_SCROLL_MS_MEDIUM;
		break;
	}
}

static void setupVerseScrollAnimation(void)
{	
	GSize contentSize = text_layer_get_content_size(s_layer_verseText);
	int16_t scrollDistance = contentSize.h - VERSE_TEXT_VISIBLE_HEIGHT;
	int16_t finalYCoord = s_verseTextYCoord - scrollDistance;
	float scrollDuration = (float)scrollDistance / s_verseScrollSettings.scrollSpeed;
	
	if (s_animation_verseScroll != NULL)
	{
		animation_destroy((Animation*)s_animation_verseScroll);
		s_animation_verseScroll = NULL;
	}
	
	if (contentSize.h <= VERSE_TEXT_VISIBLE_HEIGHT)
		s_needScroll = false;
	
	else
	{
		Layer* verseTextLayer = text_layer_get_layer(s_layer_verseText);
		GRect fromFrame = layer_get_frame(verseTextLayer);
		fromFrame.origin.y = s_verseTextYCoord; // in case we are mid scroll
		GRect toFrame = GRect(fromFrame.origin.x, finalYCoord, fromFrame.size.w, fromFrame.size.h);
		
		s_needScroll = true;

		s_animation_verseScroll = property_animation_create_layer_frame(verseTextLayer, &fromFrame, &toFrame);
	
		animation_set_duration((Animation*)s_animation_verseScroll, (uint16_t)scrollDuration);
		animation_set_curve((Animation*)s_animation_verseScroll, AnimationCurveLinear);
		animation_set_handlers( (Animation*)s_animation_verseScroll,
			(AnimationHandlers) { .stopped = (AnimationStoppedHandler)verseScrollFinishedCallback },
			NULL );	
	}
}

static void deleteInverterLayer(InverterLayer** ppLayer)
{
	if (*ppLayer != NULL)
	{
		layer_remove_from_parent((Layer*)*ppLayer);
		inverter_layer_destroy(*ppLayer);
		*ppLayer = NULL;
	}
}

static void invertColours(bool invert)
{
	if (invert)
	{
		if (s_layer_inverterMain != NULL)
			return;
		
		s_layer_inverterMain = inverter_layer_create(GRect(2, 11, 140, 132));
		s_layers_inverterTop[0] = inverter_layer_create(GRect(11, 75, 122, 1));
		s_layers_inverterTop[1] = inverter_layer_create(GRect(8, 76, 128, 1));
		s_layers_inverterTop[2] = inverter_layer_create(GRect(6, 77, 132, 1));
		s_layers_inverterTop[3] = inverter_layer_create(GRect(5, 78, 134, 1));
		s_layers_inverterTop[4] = inverter_layer_create(GRect(4, 79, 136, 2));
		s_layers_inverterTop[5] = inverter_layer_create(GRect(3, 81, 138, 3));
		s_layers_inverterBottom[0] = inverter_layer_create(GRect(3, 0, 138, 3));
		s_layers_inverterBottom[1] = inverter_layer_create(GRect(4, 3, 136, 2));
		s_layers_inverterBottom[2] = inverter_layer_create(GRect(5, 5, 134, 1));
		s_layers_inverterBottom[3] = inverter_layer_create(GRect(6, 6, 132, 1));
		s_layers_inverterBottom[4] = inverter_layer_create(GRect(8, 7, 128, 1));
		s_layers_inverterBottom[5] = inverter_layer_create(GRect(11, 8, 122, 1));
		
		layer_insert_below_sibling((Layer*)s_layer_inverterMain, (Layer*)s_layer_borderTop);

		for (uint8_t i=0; i<NUM_INVERTER_LAYERS_PER_BORDER; i++)
			layer_add_child((Layer*)s_layer_borderTop, (Layer*)s_layers_inverterTop[i]);

		for (uint8_t i=0; i<NUM_INVERTER_LAYERS_PER_BORDER; i++)
			layer_add_child((Layer*)s_layer_borderBottom, (Layer*)s_layers_inverterBottom[i]);
	}
	else
	{
		if (s_layer_inverterMain == NULL)
			return;
		
		deleteInverterLayer(&s_layer_inverterMain);

		for (uint8_t i=0; i<NUM_INVERTER_LAYERS_PER_BORDER; i++)
			deleteInverterLayer(&s_layers_inverterTop[i]);

		for (uint8_t i=0; i<NUM_INVERTER_LAYERS_PER_BORDER; i++)
			deleteInverterLayer(&s_layers_inverterBottom[i]);
	}
}

void setupVerseTextSettings(VerseTextSettings_t* pSettings)
{
	/* Available system fonts: */
	// FONT_KEY_GOTHIC_14
	// FONT_KEY_GOTHIC_14_BOLD
	// FONT_KEY_GOTHIC_18
	// FONT_KEY_GOTHIC_18_BOLD
	// FONT_KEY_GOTHIC_24
	// FONT_KEY_GOTHIC_24_BOLD
	// FONT_KEY_GOTHIC_28
	// FONT_KEY_GOTHIC_28_BOLD
		
	switch (s_settingVerseFont)
	{
	case VERSE_FONT_SMALL:
		pSettings->font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
		pSettings->padding = VERSE_TEXT_PADDING_SMALL;
		break;
	case VERSE_FONT_LARGE:
		pSettings->font = fonts_get_system_font(FONT_KEY_GOTHIC_24);
		pSettings->padding = VERSE_TEXT_PADDING_XLARGE;
		break;
	case VERSE_FONT_XLARGE:
		pSettings->font = fonts_get_system_font(FONT_KEY_GOTHIC_28);
		pSettings->padding = VERSE_TEXT_PADDING_LARGE;
		break;
	default: // go for medium in case of error
		pSettings->font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
		pSettings->padding = VERSE_TEXT_PADDING_MEDIUM;
		break;
	}
	
	return;
}

void createVerseTextLayer(char* text)
{
	bool wasHidden = true;
	GRect box = GRect(0, 0, 132, VERSE_BOX_MAX_HEIGHT);
	VerseTextSettings_t verseTextSettings = {NULL, 0};
	
	setupVerseTextSettings(&verseTextSettings);
	
	GSize contentSize = graphics_text_layout_get_content_size(text, verseTextSettings.font, box, GTextOverflowModeWordWrap, GTextAlignmentCenter);
	
	if (s_layer_verseText != NULL)
	{
		wasHidden = layer_get_hidden((Layer*)s_layer_verseText);
		text_layer_destroy(s_layer_verseText);
	}
	
	s_verseTextYCoord = VERSE_TEXT_Y_COORD - verseTextSettings.padding;
		
	s_layer_verseText = text_layer_create(GRect(6, s_verseTextYCoord, 132, (contentSize.h + verseTextSettings.padding))); // TODO macro the 132
	text_layer_set_background_color(s_layer_verseText, GColorBlack);
	text_layer_set_text_color(s_layer_verseText, GColorWhite);
	text_layer_set_text(s_layer_verseText, text);
	text_layer_set_font(s_layer_verseText, verseTextSettings.font);
	text_layer_set_text_alignment(s_layer_verseText, GTextAlignmentCenter);
	
	layer_set_hidden((Layer*)s_layer_verseText, wasHidden);
	
	layer_insert_below_sibling((Layer*)s_layer_verseText, (Layer*)s_layer_timeDisplay);
}

static void inbox_received_callback(DictionaryIterator* iterator, void* context)
{
	static char ref_buffer[REFERENCE_MAX_SIZE];
	static char text_buffer[VERSE_MAX_SIZE];
	
	bool gotRef = false;
	bool gotText = false;
	bool gotVerseFont = false;
	bool needVerseAnimationAdjust = false;

	// Read first item
	Tuple* t = dict_read_first(iterator);
	
	// For all items
	while(t != NULL)
	{
		switch(t->key)
		{
		case KEY_VERSE_REFERENCE:
			snprintf(ref_buffer, sizeof(ref_buffer), "%s", t->value->cstring);
			text_layer_set_text(s_layer_verseRef, ref_buffer);
			gotRef = true;
			break;
		case KEY_VERSE_TEXT:
			snprintf(text_buffer, sizeof(text_buffer), "%s", t->value->cstring);
			gotText = true;
			break;
		case KEY_DATE_FORMAT:
			s_settingDateFormat = t->value->uint8;
			update_date();
			break;
		case KEY_UPDATE_TIME:
			s_settingUpdateTime = t->value->uint16;
			break;
		case KEY_ENABLE_LIGHT:
			s_settingEnableLight = (t->value->uint8 != 0);
			break;
		case KEY_BT_VIBE:
			s_settingBtVibe = (t->value->uint8 != 0);
			break;
		case KEY_INVERT_COLOURS:
			s_settingInvertColours = (t->value->uint8 != 0);
			invertColours(s_settingInvertColours);
			break;
		case KEY_SHOW_AMPM:
			s_settingShowAmPm = (t->value->uint8 != 0);
			update_time(); // TODO: separate am/pm display to its own function
			break;
		case KEY_VERSE_FONT:
			s_settingVerseFont = t->value->uint16;
			gotVerseFont = true;
			break;
		case KEY_SCROLL_SPEED:
			s_settingScrollSpeed = t->value->uint16;
			setupVerseScrollSettings();
			needVerseAnimationAdjust = true;
			break;
		default:
			APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
			break;
		}
		
		t = dict_read_next(iterator);
	}

#ifdef DEMO_MODE
	strcpy(ref_buffer, DEMO_VERSE_REF);
	text_layer_set_text(s_layer_verseRef, ref_buffer);
	gotRef = true;
	
	strcpy(text_buffer, DEMO_VERSE_TEXT);
	text_layer_set_text(s_layer_verseText, text_buffer);
	gotText = true;
	adjustVerseTextPosition();
	setupVerseScrollAnimation();
#endif // DEMO_MODE	
	
	if (gotRef && gotText)
		s_gotVerse = true;
	
	if (gotText || gotVerseFont)
	{
		createVerseTextLayer(text_buffer);
		needVerseAnimationAdjust = true;
	}
	
	if (needVerseAnimationAdjust)
		setupVerseScrollAnimation();
	
	if (s_gotVerse && s_verseShown)
	{
		adjustVerseTextPosition();
		startVerseDisplayTimer();
	}
}

static void inbox_dropped_callback(AppMessageResult reason, void *context)
{
	APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox dropped: %s", translate_error(reason));
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context)
{
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox failed: %s", translate_error(reason));
	
	s_timer_requestVerseTimer = app_timer_register(UPDATE_INTERVAL, requestVerseTimerCallback, NULL);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context)
{
	APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
	
	if (s_requestSettings)
		s_requestSettings = false;
}

static void main_window_load(Window* window)
{	
	Layer* window_layer = window_get_root_layer(window);
	
	// Vertical borders
	s_bitmap_bordersVert = gbitmap_create_with_resource(RESOURCE_ID_BORDERS_VERT);
	s_bitmap_borderLeft = gbitmap_create_as_sub_bitmap(s_bitmap_bordersVert, GRect(1, 0, 4, 130));
	s_bitmap_borderRight = gbitmap_create_as_sub_bitmap(s_bitmap_bordersVert, GRect(0, 0, 4, 130));
	s_layer_borderLeft = bitmap_layer_create(GRect(0, 11, 4, 130));
	s_layer_borderRight = bitmap_layer_create(GRect(140, 11, 4, 130));
	bitmap_layer_set_bitmap(s_layer_borderLeft, s_bitmap_borderLeft);
	bitmap_layer_set_bitmap(s_layer_borderRight, s_bitmap_borderRight);
	
	// Horizontal Borders
	s_bitmap_bordersHoriz = gbitmap_create_with_resource(RESOURCE_ID_BORDERS_HORIZ);
	s_bitmap_borderTop = gbitmap_create_as_sub_bitmap(s_bitmap_bordersHoriz, GRect(0, 11, 144, 84));
	s_bitmap_borderBottom = gbitmap_create_as_sub_bitmap(s_bitmap_bordersHoriz, GRect(0, 0, 144, 84));
	s_layer_borderTop = bitmap_layer_create(GRect(0, -73, 144, 84));
	s_layer_borderBottom = bitmap_layer_create(GRect(0, 141, 144, 84));
	bitmap_layer_set_bitmap(s_layer_borderTop, s_bitmap_borderTop);
	bitmap_layer_set_bitmap(s_layer_borderBottom, s_bitmap_borderBottom);
	
	// Status Bar Icons
	s_bitmap_statusBarIcons = gbitmap_create_with_resource(RESOURCE_ID_STATUS_ICONS);
	s_bitmap_bluetoothLogo = gbitmap_create_as_sub_bitmap(s_bitmap_statusBarIcons, GRect(0, 0, 11, 16));
	s_bitmap_bluetoothStatusDisconnected = gbitmap_create_as_sub_bitmap(s_bitmap_statusBarIcons, GRect(11, 0, 7, 16));
	s_bitmap_bluetoothStatusConnected = gbitmap_create_as_sub_bitmap(s_bitmap_statusBarIcons, GRect(18, 0, 7, 16));
	s_bitmap_batteryIconBattery = gbitmap_create_as_sub_bitmap(s_bitmap_statusBarIcons, GRect(25, 0, 7, 16));
	s_bitmap_batteryIconCharging = gbitmap_create_as_sub_bitmap(s_bitmap_statusBarIcons, GRect(32, 0, 7, 16));
	s_bitmap_batteryIconPlugged = gbitmap_create_as_sub_bitmap(s_bitmap_statusBarIcons, GRect(39, 0, 7, 16));
	s_bitmap_batteryColon = gbitmap_create_as_sub_bitmap(s_bitmap_statusBarIcons, GRect(10, 0, 1, 16));
	
	// Create minitime TextLayer
	s_layer_miniTime = text_layer_create(GRect(40, -16, 64, 16));
	text_layer_set_background_color(s_layer_miniTime, GColorBlack);
	text_layer_set_text_color(s_layer_miniTime, GColorWhite);
	text_layer_set_text(s_layer_miniTime, DEFAULT_TIME_STRING);
	text_layer_set_font(s_layer_miniTime, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
	text_layer_set_text_alignment(s_layer_miniTime, GTextAlignmentCenter);
	
	// Custom font
	s_font_time = fonts_load_custom_font(resource_get_handle(FONT_TIME));
	
	// Time TextLayer
	s_layer_time = text_layer_create(GRect(0, -10, 136, 100));
	text_layer_set_background_color(s_layer_time, GColorBlack);
	text_layer_set_text_color(s_layer_time, GColorWhite);
	text_layer_set_text(s_layer_time, DEFAULT_TIME_STRING);
	text_layer_set_font(s_layer_time, s_font_time);
	text_layer_set_text_alignment(s_layer_time, GTextAlignmentCenter);
	
	// Create date TextLayer
	s_layer_date = text_layer_create(GRect(0, 100, 136, 30));
	text_layer_set_background_color(s_layer_date, GColorBlack);
	text_layer_set_text_color(s_layer_date, GColorWhite);
	text_layer_set_text(s_layer_date, DEFAULT_DATE_STRING);
	text_layer_set_font(s_layer_date, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	text_layer_set_text_alignment(s_layer_date, GTextAlignmentCenter);
	
	// Bluetooth layers
	s_layer_bluetoothGroup = layer_create(GRECT_BLUETOOTH_OUT);
	s_layer_bluetoothLogo = bitmap_layer_create(GRect(0, 0, 11, 16));
	s_layer_bluetoothStatus = bitmap_layer_create(GRect(13, 0, 7, 16));	
	bitmap_layer_set_bitmap(s_layer_bluetoothLogo, s_bitmap_bluetoothLogo);
	layer_add_child(s_layer_bluetoothGroup, (Layer*)s_layer_bluetoothLogo);
	layer_add_child(s_layer_bluetoothGroup, (Layer*)s_layer_bluetoothStatus);

	// Battery layers
	s_layer_batteryGroup = layer_create(GRECT_BATTERY_OUT);
	s_layer_batteryIcon = bitmap_layer_create(GRect(0, 0, 7, 16));
	s_layer_batteryColon = bitmap_layer_create(GRect(9, 0, 1, 16));
	s_layer_batteryLevel = text_layer_create(GRect(10, -1, 32, 16));
	bitmap_layer_set_bitmap(s_layer_batteryColon, s_bitmap_batteryColon);
	text_layer_set_background_color(s_layer_batteryLevel, GColorBlack);
	text_layer_set_text_color(s_layer_batteryLevel, GColorWhite);
	text_layer_set_font(s_layer_batteryLevel, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(s_layer_batteryLevel, GTextAlignmentCenter);
	layer_add_child(s_layer_batteryGroup, (Layer*)s_layer_batteryIcon);
	layer_add_child(s_layer_batteryGroup, (Layer*)s_layer_batteryColon);
	layer_add_child(s_layer_batteryGroup, (Layer*)s_layer_batteryLevel);	
	
	// Add elements to TimeDisplayLayer
	s_layer_timeDisplay = layer_create(GRect(4, 11, 136, 130));
	layer_add_child(s_layer_timeDisplay, text_layer_get_layer(s_layer_date));
	layer_add_child(s_layer_timeDisplay, text_layer_get_layer(s_layer_time));
	layer_add_child(s_layer_timeDisplay, s_layer_bluetoothGroup);
	layer_add_child(s_layer_timeDisplay, s_layer_batteryGroup);
	
	// Create verse reference TextLayer
	s_layer_verseRef = text_layer_create(GRect(0, 152, 144, 16));
	text_layer_set_background_color(s_layer_verseRef, GColorBlack);
	text_layer_set_text_color(s_layer_verseRef, GColorWhite);
	text_layer_set_text(s_layer_verseRef, "Verse of the day");
	text_layer_set_font(s_layer_verseRef, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
	text_layer_set_text_alignment(s_layer_verseRef, GTextAlignmentCenter);
		
	// Add layers to window
	layer_add_child(window_layer, bitmap_layer_get_layer(s_layer_borderLeft));
	layer_add_child(window_layer, bitmap_layer_get_layer(s_layer_borderRight));
	// s_layer_verseText will be added here, see below
	layer_add_child(window_layer, s_layer_timeDisplay);
	layer_add_child(window_layer, bitmap_layer_get_layer(s_layer_borderTop));
	layer_add_child(window_layer, bitmap_layer_get_layer(s_layer_borderBottom));
	layer_add_child(window_layer, text_layer_get_layer(s_layer_verseRef));
	layer_add_child(window_layer, (Layer*)s_layer_miniTime);
	
	// Create verse text TextLayer
	// NOTE: createVerseTextLayer adds layer to window behind s_layer_timeDisplay
	s_layer_verseText = NULL;
	createVerseTextLayer("Loading...\n\n(Please check data connection)");
	
	// variables to be set up later
	s_verseShown = false;
	s_gotVerse = false;
	s_needScroll = false;
	s_verseRequests = 0;	
	s_animation_verseScroll = NULL;
	s_shakeAwake = false;
	s_timer_verseDisplayTimer = NULL;
	s_timer_requestVerseTimer = NULL;
	s_timer_shakeAwake = NULL;
	s_timer_light = NULL;
	s_timer_hideStatus = NULL;
	s_animation_bluetooth = NULL;
	s_animation_battery = NULL;
	s_layer_amPm = NULL;
	s_font_amPm = NULL;
	s_layer_inverterMain = NULL;
	for (uint8_t i=0; i<NUM_INVERTER_LAYERS_PER_BORDER; i++)
		s_layers_inverterTop[i] = NULL;
	for (uint8_t i=0; i<NUM_INVERTER_LAYERS_PER_BORDER; i++)
		s_layers_inverterBottom[i] = NULL;

	initMiniTimeAnimation();
	initBorderTopAnimations();
	initBorderBottomAnimations();
	setupVerseScrollSettings();
	setupVerseScrollAnimation();
	
	updateBluetoothStatus(bluetooth_connection_service_peek());
	batteryStateHandler(battery_state_service_peek()); // causes appropriate status icons to be animated in
	
	invertColours(s_settingInvertColours);
	
	getVerse();
}

static void main_window_unload(Window* window)
{
	animation_unschedule_all(); // destroys status animations TODO: maybe others (through stopped callback)?
	invertColours(false); // deletes inverter layers
	fonts_unload_custom_font(s_font_time);
	if(s_font_amPm != NULL)
		fonts_unload_custom_font(s_font_amPm);
	text_layer_destroy(s_layer_time);
	text_layer_destroy(s_layer_date);
	text_layer_destroy(s_layer_verseRef);
	text_layer_destroy(s_layer_miniTime);
	text_layer_destroy(s_layer_verseText);
	text_layer_destroy(s_layer_batteryLevel);
	if (s_layer_amPm != NULL)
		text_layer_destroy(s_layer_amPm);
	bitmap_layer_destroy(s_layer_borderTop);
	bitmap_layer_destroy(s_layer_borderBottom);
	bitmap_layer_destroy(s_layer_borderLeft);
	bitmap_layer_destroy(s_layer_borderRight);
	bitmap_layer_destroy(s_layer_bluetoothLogo);
	bitmap_layer_destroy(s_layer_bluetoothStatus);
	bitmap_layer_destroy(s_layer_batteryIcon);
	bitmap_layer_destroy(s_layer_batteryColon);
	gbitmap_destroy(s_bitmap_bordersVert);
	gbitmap_destroy(s_bitmap_bordersHoriz);
	gbitmap_destroy(s_bitmap_borderTop);
	gbitmap_destroy(s_bitmap_borderBottom);
	gbitmap_destroy(s_bitmap_borderLeft);
	gbitmap_destroy(s_bitmap_borderRight);
	gbitmap_destroy(s_bitmap_statusBarIcons);
	gbitmap_destroy(s_bitmap_bluetoothLogo);
	gbitmap_destroy(s_bitmap_bluetoothStatusConnected);
	gbitmap_destroy(s_bitmap_bluetoothStatusDisconnected);
	gbitmap_destroy(s_bitmap_batteryColon);
	gbitmap_destroy(s_bitmap_batteryIconBattery);
	gbitmap_destroy(s_bitmap_batteryIconCharging);
	gbitmap_destroy(s_bitmap_batteryIconPlugged);
	layer_destroy(s_layer_timeDisplay);
	layer_destroy(s_layer_bluetoothGroup);
	layer_destroy(s_layer_batteryGroup);
	animation_destroy((Animation*) s_animation_miniTime);
	animation_destroy((Animation*) s_animation_borderTopCompress);
	animation_destroy((Animation*) s_animation_borderTopExpand);
	animation_destroy((Animation*) s_animation_borderBottomCompress);
	animation_destroy((Animation*) s_animation_borderBottomExpand);
	if (s_animation_verseScroll != NULL)
		animation_destroy((Animation*) s_animation_verseScroll);
}

static void writePersistentStorage(void);

static void resolvePersistantStorage(int16_t oldStorageVersion)
{
	s_requestSettings = false;
	
	// translate settings
	switch(oldStorageVersion)
	{
	case 0:
		{
			// leave s_settingDateFormat as default
			// leave s_settingShowAmPm as default
			// leave s_settingVerseFont as default
			// leave s_settingScrollSpeed as default
			// persist read other settings
			s_settingDateFormat = persist_read_int(PERSIST_KEY_DATEFORMAT);
			s_settingUpdateTime = persist_read_int(PERSIST_KEY_UPDATETIME);
			s_settingEnableLight = persist_read_bool(PERSIST_KEY_ENABLELIGHT);
			s_settingBtVibe = persist_read_bool(PERSIST_KEY_BTVIBE);
			s_settingInvertColours = persist_read_bool(PERSIST_KEY_INVERTCOLOURS);
		}
		break;
	default: // clean install of program, leave all settings as defaults
		{
			// clean install or upgrading from old version; phone's settings may be out of sync
			// need to request all settings to be sent from phone
			s_requestSettings = true;
		}
		break;
	}
		
	persist_write_int(PERSIST_KEY_STORAGEVERSION, PERSISTENT_STORAGE_VERSION);
	writePersistentStorage();
}

static void readPersistentStorage(void)
{
	// resolve persistent storage contents if necessary
	int16_t storageVersion = -1;
	if (persist_exists(PERSIST_KEY_STORAGEVERSION))
		storageVersion = persist_read_int(PERSIST_KEY_STORAGEVERSION);
	if (storageVersion != PERSISTENT_STORAGE_VERSION)
		resolvePersistantStorage(storageVersion);
	
	// assume all keys exist since we are resolved
	s_settingDateFormat = persist_read_int(PERSIST_KEY_DATEFORMAT);
	s_settingUpdateTime = persist_read_int(PERSIST_KEY_UPDATETIME);
	s_settingEnableLight = persist_read_bool(PERSIST_KEY_ENABLELIGHT);
	s_settingBtVibe = persist_read_bool(PERSIST_KEY_BTVIBE);
	s_settingInvertColours = persist_read_bool(PERSIST_KEY_INVERTCOLOURS);
	s_settingShowAmPm = persist_read_bool(PERSIST_KEY_SHOWAMPM);
	s_settingVerseFont = persist_read_int(PERSIST_KEY_VERSEFONT);
	s_settingScrollSpeed = persist_read_int(PERSIST_KEY_SCROLLSPEED);
}

void writePersistentStorage(void)
{	
	if ( !persist_exists(PERSIST_KEY_DATEFORMAT) || (persist_read_int(PERSIST_KEY_DATEFORMAT) != s_settingDateFormat) )
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Persisting DateFormat: %d", s_settingDateFormat);
		persist_write_int(PERSIST_KEY_DATEFORMAT, s_settingDateFormat);
	}
	
	if ( !persist_exists(PERSIST_KEY_UPDATETIME) || (persist_read_int(PERSIST_KEY_UPDATETIME) != s_settingUpdateTime) )
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Persisting UpdateTime: %d", s_settingUpdateTime);
		persist_write_int(PERSIST_KEY_UPDATETIME, s_settingUpdateTime);
	}
	
	if ( !persist_exists(PERSIST_KEY_ENABLELIGHT) || (persist_read_bool(PERSIST_KEY_ENABLELIGHT) != s_settingEnableLight) )
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Persisting EnableLight: %d", s_settingEnableLight);
		persist_write_bool(PERSIST_KEY_ENABLELIGHT, s_settingEnableLight);
	}
	
	if ( !persist_exists(PERSIST_KEY_BTVIBE) || (persist_read_bool(PERSIST_KEY_BTVIBE) != s_settingBtVibe) )
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Persisting BtVibe: %d", s_settingBtVibe);
		persist_write_bool(PERSIST_KEY_BTVIBE, s_settingBtVibe);
	}
	
	if ( !persist_exists(PERSIST_KEY_INVERTCOLOURS) || (persist_read_bool(PERSIST_KEY_INVERTCOLOURS) != s_settingInvertColours) )
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Persisting InvertColours: %d", s_settingInvertColours);
		persist_write_bool(PERSIST_KEY_INVERTCOLOURS, s_settingInvertColours);
	}
	
	if ( !persist_exists(PERSIST_KEY_SHOWAMPM) || (persist_read_bool(PERSIST_KEY_SHOWAMPM) != s_settingShowAmPm) )
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Persisting ShowAmPm: %d", s_settingShowAmPm);
		persist_write_bool(PERSIST_KEY_SHOWAMPM, s_settingShowAmPm);
	}
	
	if ( !persist_exists(PERSIST_KEY_VERSEFONT) || (persist_read_int(PERSIST_KEY_VERSEFONT) != s_settingVerseFont) )
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Persisting VerseFont: %d", s_settingVerseFont);
		persist_write_int(PERSIST_KEY_VERSEFONT, s_settingVerseFont);
	}
	
	if ( !persist_exists(PERSIST_KEY_SCROLLSPEED) || (persist_read_int(PERSIST_KEY_SCROLLSPEED) != s_settingScrollSpeed) )
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Persisting ScrollSpeed: %d", s_settingScrollSpeed);
		persist_write_int(PERSIST_KEY_SCROLLSPEED, s_settingScrollSpeed);
	}
}

static void init()
{
	readPersistentStorage();
	
	// Create main Window element and assign to pointer
	s_main_window = window_create();
	window_set_background_color(s_main_window, GColorBlack);

	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});

	// Show the Window on the watch, with animated=true
	window_stack_push(s_main_window, true);
		
	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
	accel_tap_service_subscribe(tap_handler);
	bluetooth_connection_service_subscribe(bluetoothConnectionHandler);
	battery_state_service_subscribe(batteryStateHandler);
	
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);		
	
	int ret = app_message_open(app_message_inbox_size_maximum(),  APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "app_message_open returned %s", translate_error(ret));
	
	update_time();
	update_date();
}

static void deinit()
{
	window_destroy(s_main_window);
	
	tick_timer_service_unsubscribe();
	accel_tap_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	battery_state_service_unsubscribe();
	
	writePersistentStorage();
}

int main(void)
{
	init();
	app_event_loop();
	deinit();
}
