#include <pebble.h>
	
//#define ENABLE_TIME_BAR
//#define DEMO_MODE
#define DEMO_TIME_24H "10:45"
#define DEMO_TIME_12H "08:30 PM"
#define DEMO_DATE "Sat 6 Dec"
#define DEMO_VERSE_REF "John 10:10"
#define DEMO_VERSE_TEXT "The thief comes only to steal and kill and destroy; I have come that they may have life, and have it to the full."

#define KEY_VERSE_REFERENCE 0
#define KEY_VERSE_TEXT 1
#define KEY_BAR_START 2
#define KEY_BAR_END 3
#define KEY_UPDATE_TIME 4
#define KEY_ENABLE_LIGHT 5
#define KEY_BT_VIBE 6
#define KEY_INVERT_COLOURS 7
#define PERSIST_KEY_BARSTART 0
#define PERSIST_KEY_BAREND 1
#define PERSIST_KEY_UPDATETIME 2
#define PERSIST_KEY_ENABLELIGHT 3
#define PERSIST_KEY_BTVIBE 4
#define PERSIST_KEY_INVERTCOLOURS 5
	
#define DEFAULT_BAR_START 480 // 8am
#define DEFAULT_BAR_END 0 // 12am
#define DEFAULT_UPDATE_TIME 360 // 6am
#define DEFAULT_ENABLE_LIGHT true
#define DEFAULT_BT_VIBE true
#define DEFAULT_INVERT_COLOURS false

#define DEFAULT_DATE_STRING "DAY ## MTH"
#define DEFAULT_TIME_STRING "HH:MMxx"
#define VERSE_ANIMATION_DURATION 400
#define VERSE_WAIT_TIME_SCROLL 6000
#define VERSE_WAIT_TIME_NOSCROLL 15000
#define MINUTES_PER_DAY 1440
#define VERSE_TEXT_VISIBLE_HEIGHT 114
#define SCROLL_SPEED 0.008f // Pixels per Millisecond
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

#define GRECT_BLUETOOTH_OUT GRect(-20, 90, 20, 16)
#define GRECT_BLUETOOTH_IN GRect(58, 90, 20, 16)
#define GRECT_BLUETOOTH_OFFSET GRect(26, 90, 20, 16)
#define GRECT_BATTERY_OUT GRect(168, 90, 42, 16)
#define GRECT_BATTERY_IN GRect(49, 90, 42, 16)
#define GRECT_BATTERY_OFFSET GRect(76, 90, 42, 16)

#define FONT_TIME RESOURCE_ID_FONT_TIME_96
#define FONT_AMPM RESOURCE_ID_FONT_AMPM_38
	
#define NUM_INVERTER_LAYERS_PER_BORDER 6

static const uint8_t VERSE_TEXT_Y_COORD = 23;

static Window* s_main_window;
static TextLayer* s_layer_time;
static TextLayer* s_layer_date;
static TextLayer* s_layer_verseRef;
static TextLayer* s_layer_amPm;
static Layer* s_layer_timeBar;
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
static uint16_t s_barLength;

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

static float s_barPxPerMinDay;
static float s_barPxPerMinNight;
static bool s_isDayTime;
static bool s_needScroll;
static bool s_bluetoothConnected;
static uint8_t s_batteryStatus;
static uint8_t s_batteryLevel;
static bool s_shakeAwake;

// settable through settings
static uint16_t s_settingBarStart;
static uint16_t s_settingBarEnd;
static uint16_t s_settingUpdateTime;
static uint16_t s_settingEnableLight;
static bool s_settingBtVibe;
static bool s_settingInvertColours;

static GFont* s_font_time;
static GFont* s_font_amPm;

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
	
	uint8_t yCoord = VERSE_TEXT_Y_COORD;
	
	if (contentSize.h <= VERSE_TEXT_VISIBLE_HEIGHT)
		yCoord += ((VERSE_TEXT_VISIBLE_HEIGHT - contentSize.h) >> 1);
	
	layer_set_frame(text_layer_get_layer(s_layer_verseText), GRect(4, yCoord, 136, 300));
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
		s_timer_verseDisplayTimer = app_timer_register(VERSE_WAIT_TIME_SCROLL, scheduleScrollAnimation, NULL);
	else
		s_timer_verseDisplayTimer = app_timer_register(VERSE_WAIT_TIME_NOSCROLL, timerCbHideVerse, NULL);	
}

static void verseScrollFinishedCallback(Animation* animation, bool finished, void* context)
{	
	if (finished)
	{
		cancelTimer(s_timer_verseDisplayTimer);
		s_timer_verseDisplayTimer = app_timer_register(VERSE_WAIT_TIME_SCROLL, timerCbHideVerse, NULL);
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

// arguments are times expressed in minutes from midnight, order is important
static uint16_t diffTimes(uint16_t startTime, uint16_t endTime)
{
	int16_t mins = endTime - startTime;
	
	if (mins < 0)
		mins += MINUTES_PER_DAY;
	
	return (uint16_t)mins;
}
#ifdef ENABLE_TIME_BAR
void updateTimeBar(uint16_t timeNow)
{
	uint16_t minsElapsed;
		
	if (s_settingBarStart < s_settingBarEnd)
		s_isDayTime = (timeNow >= s_settingBarStart) && (timeNow <= s_settingBarEnd);
	else
		s_isDayTime = (timeNow >= s_settingBarStart) || (timeNow <= s_settingBarEnd);
	
	if (s_isDayTime)
	{
		minsElapsed = diffTimes(s_settingBarStart, timeNow);
		s_barLength = s_barPxPerMinDay * (float)minsElapsed;
	}
	else
	{
		minsElapsed = diffTimes(s_settingBarEnd, timeNow);
		s_barLength = s_barPxPerMinNight * (float)minsElapsed + 1;
	}
		
	layer_mark_dirty(s_layer_timeBar);
}
#endif //ENABLE_TIME_BAR

static void sendAppMessage(void)
{
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	
	if (iter)
	{
		//dict_write_cstring(iter, KEY_VERSE_REFERENCE, "placeholder");
		app_message_outbox_send();
	}
}

static void requestVerseTimerCallback(void* data)
{
	if (!s_gotVerse)
	{
		sendAppMessage();
		s_verseRequests += 1;
		
		if (s_verseRequests <= MAX_VERSE_REQUESTS_PER_HOUR) 
			s_timer_requestVerseTimer = app_timer_register(UPDATE_INTERVAL, requestVerseTimerCallback, NULL);
		else
		{
			s_verseRequests = 0;
			s_timer_requestVerseTimer = NULL;
		}
	}
}

static void getVerse(void)
{
	s_gotVerse = false;
	s_verseRequests = 0;
	
	cancelTimer(s_timer_requestVerseTimer);
	s_timer_requestVerseTimer = app_timer_register(UPDATE_INTERVAL, requestVerseTimerCallback, NULL);
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
		
		uint8_t amPmYCoord = 0;
		if (timeBuf12h[6] == 'P')
			amPmYCoord = 49;
		
		GSize timeSize = text_layer_get_content_size(s_layer_time);
		
		int16_t timeXCoord = (112 - timeSize.w) >> 1; // centralise with space for amPm
		layer_set_frame((Layer*)s_layer_time, GRect(timeXCoord, -10, (136 - timeXCoord), 100));
		
		layer_set_frame((Layer*)s_layer_amPm, GRect((timeXCoord + timeSize.w), amPmYCoord, 24, 40));
	}
		
	uint16_t minsSinceMidnight = (tick_time->tm_hour * 60) + tick_time->tm_min;
	
	if ( (minsSinceMidnight == s_settingUpdateTime)
		|| (!s_gotVerse && (tick_time->tm_min == 0)) )
		getVerse();
	
#ifdef ENABLE_TIME_BAR
	updateTimeBar(minsSinceMidnight);
#endif
}

#ifdef ENABLE_TIME_BAR
void initTimeBar(void)
{
	uint16_t minsInDay, minsInNight;
	
	if (s_settingBarStart == s_settingBarEnd)
	{
		minsInDay = MINUTES_PER_DAY;
		minsInNight = 1; // prevent irrational behaviour, although it will never be night
	}
	else
	{
		minsInDay = diffTimes(s_settingBarStart, s_settingBarEnd);
		minsInNight = MINUTES_PER_DAY - minsInDay;
	}
	
	s_barPxPerMinDay = 96.0f / (float)minsInDay; // 96 is inner bar max length	
	s_barPxPerMinNight = 96.0f / (float)minsInNight;
	
	time_t epochTime = time(NULL); 
	struct tm *pTime = localtime(&epochTime);
	uint16_t minsSinceMidnight = (pTime->tm_hour * 60) + pTime->tm_min;
	
	updateTimeBar(minsSinceMidnight);
}
#endif //ENABLE_TIME_BAR

static void update_date()
{
	time_t temp = time(NULL); 
	struct tm *tick_time = localtime(&temp);

	static char date_buffer[] = DEFAULT_DATE_STRING;

	strftime(date_buffer, sizeof(DEFAULT_DATE_STRING), "%a %e %b", tick_time);
	
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
		static const uint32_t const btConnPattern[] = { 100, 300, 100 };
		vibePattern = (VibePattern) {
			.durations = btConnPattern,
			.num_segments = ARRAY_LENGTH(btConnPattern)
		};
	}
	else
	{
		static const uint32_t const btDisconPattern[] = { 100, 300, 400 };
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

#ifdef ENABLE_TIME_BAR	
static void draw_time_bar(Layer *this_layer, GContext *ctx)
{
	GRect canvasBounds = layer_get_bounds(this_layer);
	
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_draw_rect(ctx, canvasBounds);
	
	GRect barBounds;
	barBounds.origin = (GPoint){ (canvasBounds.origin.x + 2), (canvasBounds.origin.y + 2) };
	barBounds.size = (GSize){ (canvasBounds.size.w - 4), (canvasBounds.size.h - 4) };
	
	if (s_isDayTime)
	{
		GRect barToDraw = GRect(barBounds.origin.x, barBounds.origin.y, s_barLength, barBounds.size.h);
		graphics_fill_rect(ctx, barToDraw, 0, GCornerNone);
	}
	else
	{
		GPoint lineStart = (GPoint){ (barBounds.origin.x + s_barLength), barBounds.origin.y };
		GPoint lineEnd = (GPoint){ lineStart.x, (barBounds.origin.y + barBounds.size.h - 1) };
		graphics_draw_line(ctx, lineStart, lineEnd);
	}
}
#endif // ENABLE_TIME_BAR
	
static void setupVerseScrollAnimation(void)
{
	GSize contentSize = text_layer_get_content_size(s_layer_verseText);
	int16_t scrollDistance = contentSize.h - VERSE_TEXT_VISIBLE_HEIGHT;
	int16_t finalYCoord = VERSE_TEXT_Y_COORD - scrollDistance;
	float scrollDuration = (float)scrollDistance / SCROLL_SPEED;
	
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
		GRect toFrame = GRect(4, finalYCoord, 136, 300);
		
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

static void inbox_received_callback(DictionaryIterator* iterator, void* context)
{
	static char ref_buffer[REFERENCE_MAX_SIZE];
	static char text_buffer[VERSE_MAX_SIZE];
	
	bool gotRef = false;
	bool gotText = false;
	
#ifdef ENABLE_TIME_BAR
	bool gotBarTime = false;
#endif

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
			text_layer_set_text(s_layer_verseText, text_buffer);
			gotText = true;
			adjustVerseTextPosition();
			setupVerseScrollAnimation();
			break;
#ifdef ENABLE_TIME_BAR
		case KEY_BAR_START:
			if (s_settingBarStart != t->value->uint16)
			{
				s_settingBarStart = t->value->uint16;
				gotBarTime = true;
			}
			break;
		case KEY_BAR_END:
			if (s_settingBarEnd != t->value->uint16)
			{
				s_settingBarEnd = t->value->uint16;
				gotBarTime = true;
			}
			break;
#endif // ENABLE_TIME_BAR
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
	
	if (s_gotVerse && s_verseShown)
		startVerseDisplayTimer();
#ifdef ENABLE_TIME_BAR	
	if (gotBarTime)
		initTimeBar();
#endif
}

static void inbox_dropped_callback(AppMessageResult reason, void *context)
{
	APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox dropped: %s", translate_error(reason));
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context)
{
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox failed: %s", translate_error(reason));
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context)
{
	APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
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
	
	// Create canvas Layer
	s_layer_timeBar = layer_create(GRect(18, 119, 100, 11));
#ifdef ENABLE_TIME_BAR	
	layer_set_update_proc(s_layer_timeBar, draw_time_bar);
#endif
	
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
#ifdef ENABLE_TIME_BAR	
	layer_add_child(s_layer_timeDisplay, s_layer_timeBar);
#endif
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
	
	// Create verse text TextLayer
	s_layer_verseText = text_layer_create(GRect(4, VERSE_TEXT_Y_COORD, 136, 300)); // four px padding above text. 114 px visible (height)
	text_layer_set_background_color(s_layer_verseText, GColorBlack);
	text_layer_set_text_color(s_layer_verseText, GColorWhite);
	text_layer_set_text(s_layer_verseText, "Loading...");
	text_layer_set_font(s_layer_verseText, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(s_layer_verseText, GTextAlignmentCenter);
	layer_set_hidden((Layer*)s_layer_verseText, true);
	adjustVerseTextPosition();
		
	// Add layers to window
	layer_add_child(window_layer, bitmap_layer_get_layer(s_layer_borderLeft));
	layer_add_child(window_layer, bitmap_layer_get_layer(s_layer_borderRight));
	layer_add_child(window_layer, text_layer_get_layer(s_layer_verseText));
	layer_add_child(window_layer, s_layer_timeDisplay);
	layer_add_child(window_layer, bitmap_layer_get_layer(s_layer_borderTop));
	layer_add_child(window_layer, bitmap_layer_get_layer(s_layer_borderBottom));
	layer_add_child(window_layer, text_layer_get_layer(s_layer_verseRef));
	layer_add_child(window_layer, (Layer*)s_layer_miniTime);
	
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

#ifdef ENABLE_TIME_BAR	
	initTimeBar();
#endif
	initMiniTimeAnimation();
	initBorderTopAnimations();
	initBorderBottomAnimations();
	
	updateBluetoothStatus(bluetooth_connection_service_peek());
	updateBatteryState(battery_state_service_peek());
	
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
	layer_destroy(s_layer_timeBar);
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

static void readPersistentStorage(void)
{	
	if (persist_exists(PERSIST_KEY_BARSTART))
		s_settingBarStart = persist_read_int(PERSIST_KEY_BARSTART);
	else
		s_settingBarStart = DEFAULT_BAR_START;
	
	if (persist_exists(PERSIST_KEY_BAREND))
		s_settingBarEnd = persist_read_int(PERSIST_KEY_BAREND);
	else
		s_settingBarEnd = DEFAULT_BAR_END;
	
	if (persist_exists(PERSIST_KEY_UPDATETIME))
		s_settingUpdateTime = persist_read_int(PERSIST_KEY_UPDATETIME);
	else
		s_settingUpdateTime = DEFAULT_UPDATE_TIME;
	
	if (persist_exists(PERSIST_KEY_ENABLELIGHT))
		s_settingEnableLight = persist_read_bool(PERSIST_KEY_ENABLELIGHT);
	else
		s_settingEnableLight = DEFAULT_ENABLE_LIGHT;
	
	if (persist_exists(PERSIST_KEY_BTVIBE))
		s_settingBtVibe = persist_read_bool(PERSIST_KEY_BTVIBE);
	else
		s_settingBtVibe = DEFAULT_BT_VIBE;
	
	if (persist_exists(PERSIST_KEY_INVERTCOLOURS))
		s_settingInvertColours = persist_read_bool(PERSIST_KEY_INVERTCOLOURS);
	else
		s_settingInvertColours = DEFAULT_INVERT_COLOURS;
}

static void writePersistentStorage(void)
{
	if ( !persist_exists(PERSIST_KEY_BARSTART) || (persist_read_int(PERSIST_KEY_BARSTART) != s_settingBarStart) )
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Persisting BarStart: %d", s_settingBarStart);
		persist_write_int(PERSIST_KEY_BARSTART, s_settingBarStart);
	}
	
	if ( !persist_exists(PERSIST_KEY_BAREND) || (persist_read_int(PERSIST_KEY_BAREND) != s_settingBarEnd) )
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Persisting BarEnd: %d", s_settingBarEnd);
		persist_write_int(PERSIST_KEY_BAREND, s_settingBarEnd);
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
