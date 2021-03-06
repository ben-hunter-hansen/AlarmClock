/*
 Alarm Clock
 Author: Ben Hansen

 The circuit:
 *
 * Push button to digital pin 6
 * Push button to digital pin 7
 * Push button to digital pin 13
 * Piezo to digital pin 8
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 10
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 9
 * LCD D7 pin to digital pin 2
 * LCD V0 pin to PWM pin 3
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 *
 */

#include <LiquidCrystal.h>
#include <alarm.h>
#include <Time.h>

LiquidCrystal lcd(12, 10, 5, 4, 9, 2);

view_mode gCurrentView;
time_info gAlarmTime;
time_info gSetTime;
field_type gSelectedField;

void setup() {

	pinMode(SELECT_SWITCH, INPUT);
	pinMode(ADJUST_SWITCH, INPUT);
	pinMode(PAGE_SWITCH, INPUT);

	time_t init = now();

	gCurrentView = V_DEFAULT;
	gSelectedField = NONE;

	lcd.begin( LCD_COLS, LCD_ROWS);
	analogWrite(LCD_CONTRAST_PIN, LCD_CONTRAST);

	seedClock(SEED_TIME, &gAlarmTime, &gSetTime);

	lcd.print(dateFormatStr(init));

	lcd.setCursor(0, 1);
	lcd.print(timeFormatStr(init));

}

void loop() {

	byte switchState = checkSwitchEvent();
	int adjustment = 0;

	// Handle user input
	switch (switchState) {
	case ADJUST_SWITCH:
		adjustment = isViewEditable(gCurrentView) ? 1 : 0;
		break;
	case SELECT_SWITCH:
		gSelectedField = nextField(gCurrentView, gSelectedField);
		break;
	case PAGE_SWITCH:
		lcd.noBlink();
		lcd.clear();
		gSelectedField = NONE;
		gCurrentView = nextView(gCurrentView);
		break;
	}

	// Update and collect system time info
	tick();
	time_t current = now();

	// See if any changes were requested and do updates
	bool hadChanges = adjustment && (gSelectedField != NONE);

	if (hadChanges && (gCurrentView != V_DEFAULT)) {
		if (gCurrentView == V_ALARMSET) {
			timeAdjustment(gSelectedField, &gAlarmTime);
		} else {
			timeAdjustment(gSelectedField, &gSetTime);
			setClockTime(gSetTime);
		}
	}

	// Render the updated view to the screen
	switch (gCurrentView) {
	case V_TIMESET:
		gSelectedField = DEF_IF_NONE(gSelectedField,HOUR);
		renderTimeset(lcd, gSetTime, gSelectedField);
		break;
	case V_DATESET:
		gSelectedField = DEF_IF_NONE(gSelectedField,WDAY);
		renderDateset(lcd, gSetTime, gSelectedField);
		break;
	case V_ALARMSET:
		gSelectedField = DEF_IF_NONE(gSelectedField,HOUR);
		renderAlarmset(lcd, gAlarmTime, gSelectedField);
		break;
	default:
		renderDefault(lcd, current);
	}

	//If the time is right, sound the alarm.
	if(isAlarmTime(current, gAlarmTime)) {
		tone(ALARM_PIN,ALARM_TONE);
	} else {
		noTone(ALARM_PIN);
	}
}

