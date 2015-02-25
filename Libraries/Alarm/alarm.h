#ifndef ALARM_H
#define ALARM_H

#include <Time.h>
#include <LiquidCrystal.h>

/*
 * 	Alarm clock defines and constants
 */

/* Helper macros */
#define INC(x,y,z) fieldIncrement(x,y,z)
#define HR12(x) (x > 12) ? x - 12 : x
#define LZ(x) leadingZero(x)
#define STR_AMPM(x) isAM(x) ? "AM" : "PM"

#define INPUT			0x0
#define OUTPUT			0x1

#define LCD_COLS 		16
#define LCD_ROWS 		2
#define LCD_CONTRAST 	50

#define HOUR_COL 		0
#define MINUTE_COL 		3
#define AMPM_COL 		6
#define WDAY_COL 		0
#define MONTH_COL 		4
#define DAY_COL 		8
#define YEAR_COL 		11

#define N_FIELD_TYPES	7

const byte LCD_CONTRAST_PIN = 9;
const byte ADJUST_SWITCH = 6;
const byte SELECT_SWITCH = 7;
const byte PAGE_SWITCH = 13;

/*
 *  - System time reference table -
 *	millis		seconds		adjustment
 *	1000		1			0.5
 *	500			0.5			0.25
 *	250			0.25		0.125
 *	125			0.125		0.0625
 */
#define SYS_TIME_ADJUSTMENT 0.0625

/* Alarm clock structures and enums */
typedef struct TIME_INFO {
	int Second;
	int Minute;
	int Hour;
	int Wday;
	int Day;
	int Month;
	int Year;
	String AmPm;
} time_info;

typedef enum VIEW_MODE {
	V_DEFAULT, V_DATESET, V_TIMESET, V_ALARMSET
} view_mode;

typedef enum FIELD_TYPE {
	HOUR, MINUTE, AMPM, WDAY, MONTH, DAY, YEAR, NONE
} field_type;

typedef struct FIELD_RANGE {
	int High;
	int Low;
} field_range;

typedef struct FIELD_DATA {
	field_type FieldType;
	int Column;
	field_range Range;
} field_data;

const field_data EDITABLE_FIELDS[N_FIELD_TYPES] = {
		{ HOUR, HOUR_COL, { 23, 0 } }, { MINUTE, MINUTE_COL, { 60, 1 } }, {
				AMPM, AMPM_COL, { } }, { WDAY, WDAY_COL, { 7, 1 } }, { MONTH,
				MONTH_COL, { 12, 1 } }, { DAY, DAY_COL, { 31, 1 } }, { YEAR,
				YEAR_COL, { 2020, 2015 } } };

/* Default system time seed */
const time_info SEED_TIME = { 0, 30, 12, 2, 23, 2, 2015, "PM" };

/*
 * 	Printable date strings who's indices
 * 	correspond with the integer values
 * 	returned by Arduino's time library.
 *
 * 	Ex. Sunday is 1, January is 1, etc.
 */
const String STR_WEEKDAY[] = { "err", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri",
		"Sat" };

const String STR_MONTH[] = { "err", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

/* View rendering functions		*/
void renderDefault(LiquidCrystal lcd, time_t to_render);
void renderTimeset(LiquidCrystal lcd, time_info to_render, field_type selected);
void renderDateset(LiquidCrystal lcd, time_info to_render, field_type selected);
void renderAlarmset(LiquidCrystal lcd, time_info to_render,
		field_type selected);

/* Time controlling functions	*/
void seedClock(time_info seed, time_info * alarm, time_info * set);
void tick();
void timeAdjustment(field_type field, time_info * t);
void setClockTime(time_info set);

/* Input event functions		*/
byte checkSwitchEvent();
bool isViewEditable(view_mode current);
view_mode nextView(view_mode current);
field_type nextField(view_mode current, field_type selected);

/* Utility functions 			*/
int calcTemp(const int sensorValue);
int getColumn(field_type f);
int nextEnum(const int base, const int last, const int actual);
int fieldIncrement(const int value, const int high, const int low);
String dateFormatStr(time_t now);
String dateInfoFormatStr(time_info t);
String timeFormatStr(time_t now);
String timeInfoFormatStr(time_info t);
String leadingZero(const int n);
String dateStrBuilder(String w, String m, String d, String y);
String timeStrBuilder(String h, String m, String a);
String timeStrBuilder(String h, String m, String s, String a);
void matchAndAdjust(field_data match, time_info * toAdjust);
void verifyAdjustment(time_info * toVerify);
#endif
