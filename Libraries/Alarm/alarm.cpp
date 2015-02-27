/*
 * 	alarm.cpp
 * 	Author: Ben Hansen
 *
 * 	Function definitions for the alarm library.
 */

#include "Arduino.h"
#include <Time.h>
#include <LiquidCrystal.h>
#include "alarm.h"

/*
 *	Seed the alarm clock with the date/time specified
 *	in the header file.
 *
 *	TODO: Parameterize function with date/time
 *	structure from a synchronization utility
 *	such as WIFI, BT, or serial port.
 */
void seedClock(time_info seed, time_info * alarm, time_info * set) {

	setTime(seed.Hour, seed.Minute, seed.Second, seed.Day, seed.Month,
			seed.Year);

	*alarm = *set = seed;
}

/*
 * 	Renders the default alarm clock view
 *
 * 	@param lcd			LCD Object.
 * 	@param to_render	A time.
 */
void renderDefault(LiquidCrystal lcd, time_t to_render) {

	lcd.setCursor(0, 0);
	lcd.print(dateFormatStr(to_render));

	lcd.setCursor(0, 1);
	lcd.print(timeFormatStr(to_render));
}

/*
 * 	Renders the time set view
 *
 * 	@param lcd			LCD Object.
 * 	@param selected 	Currently selected field.
 * 	@param to_render	A time.
 */
void renderTimeset(LiquidCrystal lcd, time_info to_render,
		field_type selected) {

	lcd.setCursor(0, 0);
	lcd.print("Set clock time");

	lcd.setCursor(0, 1);
	lcd.print(timeInfoFormatStr(to_render));

	if (selected != NONE) {
		lcd.setCursor(getColumn(selected), 1);
		lcd.blink();
	}
}

/*
 * 	Renders the date set view
 *
 * 	@param lcd			LCD Object.
 * 	@param selected 	Currently selected field.
 * 	@param to_render	A time.
 */
void renderDateset(LiquidCrystal lcd, time_info to_render,
		field_type selected) {

	lcd.setCursor(0, 0);
	lcd.print("Set clock date");

	lcd.setCursor(0, 1);
	lcd.print(dateInfoFormatStr(to_render));

	if (selected != NONE) {
		lcd.setCursor(getColumn(selected), 1);
		lcd.blink();
	}
}

/*
 * 	Renders the alarm set view
 *
 * 	@param lcd			LCD Object.
 * 	@param to_render	Alarm time.
 */
void renderAlarmset(LiquidCrystal lcd, time_info to_render,
		field_type selected) {
	lcd.setCursor(0, 0);
	lcd.print("Set alarm time");

	lcd.setCursor(0, 1);
	lcd.print(timeInfoFormatStr(to_render));

	if (selected != NONE) {
		lcd.setCursor(getColumn(selected), 1);
		lcd.blink();
	}
}
/*
 * 	Dirty way of incrementing the clock.
 * 	The value of SYS_TIME_ADJUSTMENT is 0.0625,
 * 	because the adjustTime() method defined in
 * 	Arduino's Time library is non-intuitive.
 *
 *	Consult the header file for more info.
 */
void tick() {
	delay(125);
	adjustTime( SYS_TIME_ADJUSTMENT);
}
/*
 * Increments a date/time field value stored
 * in a time info structure.
 *
 * @param field	The field to increment.
 * @param t		Time info pointer.
 */
void timeAdjustment(field_type field, time_info *t) {

	for (int i = 0; i < N_FIELD_TYPES; i++) {
		if (EDITABLE_FIELDS[i].FieldType == field) {
			field_data data = EDITABLE_FIELDS[i];
			matchAndAdjust(data, t);
		}
	}
}
/*
 * 	Sets the system time based on values
 * 	keyed in by the user.
 *
 * 	@param set	A time.
 */
void setClockTime(time_info set) {

	setTime(set.Hour, set.Minute, set.Second, set.Day, set.Month, set.Year);
}
/*
 * 	Checks the current time against the
 * 	set alarm time.
 *
 * 	@param current	The current time.
 * 	@param alarm	The alarm time.
 * 	@return			True if args are equal, else false.
 */
bool isAlarmTime(time_t current, time_info alarm) {
	bool hoursEqual = hour(current) == alarm.Hour;
	bool minsEqual = minute(current) == alarm.Minute;
	return hoursEqual && minsEqual;
}
/*
 * 	Checks for an active switch and returns
 * 	a value which identifies the active switch or
 * 	lack thereof.
 *
 * 	@return			Active switch indicator, else zero.
 */
byte checkSwitchEvent() {
	byte which = 0;

	if (digitalRead(ADJUST_SWITCH)) {
		which = ADJUST_SWITCH;
	} else if (digitalRead(SELECT_SWITCH)) {
		which = SELECT_SWITCH;
	} else if (digitalRead(PAGE_SWITCH)) {
		which = PAGE_SWITCH;
	}

	return which;
}

/*
 * 	Decide if the user can edit things in the view
 *
 * 	@param current	Current view mode.
 * 	@return			true if editing is allowed, else false
 */
bool isViewEditable(view_mode current) {
	if (current == V_TIMESET || current == V_ALARMSET || current == V_DATESET) {
		return true;
	} else {
		return false;
	}
}
/*
 * 	Returns the next view mode based
 * 	on which one is currently active.
 *
 * 	@param current	The currently active view.
 * 	@return			The next view.
 */
view_mode nextView(view_mode current) {
	return (view_mode) nextEnum(V_DEFAULT, V_ALARMSET, current);
}
/*
 * 	Returns the next field selection based on
 * 	the current view mode, and the currently
 * 	selected field type.
 *
 * 	@param current	The currently active view.
 * 	@param selected The currently selected field.
 * 	@return			The next field selection.
 */
field_type nextField(view_mode current, field_type selected) {
	field_type next = NONE;

	if (current == V_DATESET) {
		next = (field_type) nextEnum(WDAY, YEAR, selected);
	} else if (current != V_DEFAULT) {
		next = (field_type) nextEnum(HOUR, AMPM, selected);
	}
	return next;
}
/*
 * 	Utility function for calculating degrees
 * 	fahrenheit.
 *
 * 	@param sensorValue	The value of temperature sensor.
 * 	@return				The calculated temperature.
 */
int calcTemp(const int sensorValue) {
	float volts = (sensorValue / 1024.0) * 5.0;
	float celsius = (volts - 0.5) * 100;
	return trunc(celsius * 1.8) + 32;
}

/*
 * 	Utility function for converting the date
 * 	information from parameter 'now' into a
 * 	printable string.
 *
 * 	Ex. Sun Feb 22 2015
 *
 * 	@param now	A timestamp.
 * 	@return		A formatted date string.
 */
String dateFormatStr(time_t now) {

	int theDay = day(now);
	String theWday = STR_WEEKDAY[weekday(now)];
	String theMonth = STR_MONTH[month(now)];
	String theYear = String(year(now));

	return dateStrBuilder(theWday, theMonth, LZ(theDay), theYear);
}

/*
 * 	Utility function for converting the time
 * 	information from parameter 'now' into a
 * 	printable string. (H/mm/ss)
 *
 * 	Ex. 9:30:00 PM
 *
 * 	@param now	A timestamp.
 * 	@return		A formatted time string.
 */
String timeFormatStr(time_t now) {

	int hr = hourFormat12(now);
	int min = minute(now);
	int sec = second(now);

	return timeStrBuilder(LZ(hr), LZ(min), LZ(sec), STR_AMPM(now));
}

/*
 * 	Utility function for converting a
 * 	time info structure to a printable
 * 	date format.
 *
 * 	@param t	The time info.
 * 	@return		The formatted string.
 */
String dateInfoFormatStr(time_info t) {

	int theDay = t.Day;
	String theWday = STR_WEEKDAY[t.Wday];
	String theMonth = STR_MONTH[t.Month];
	String theYear = String(t.Year);

	return dateStrBuilder(theWday, theMonth, LZ(theDay), theYear);
}

/*
 * 	Utility function for converting a
 * 	time info structure to a printable
 * 	time format.
 *
 * 	@param t	The time info.
 * 	@return		The formatted string.
 */
String timeInfoFormatStr(time_info t) {

	int hr = HR12(t.Hour) == 0 ? 12 : HR12(t.Hour);
	int min = t.Minute;

	return timeStrBuilder(LZ(hr), LZ(min), t.AmPm);
}

/*
 * 	Utility function for converting
 * 	a number to a string where a
 * 	leading zero is required for
 * 	formatting purposes
 *
 * 	@param n	The number to convert
 * 	@return		The string representation of n.
 */
String leadingZero(const int n) {
	String s = "";
	n < 10 ? s.concat('0') : 0;
	s.concat(n);
	return s;
}

/*
 * 	Utility function for building up a formatted time
 * 	string.  Exists because doing string concatenation
 * 	with the String object is ugly as hell.
 *
 * 	@param h	The hour.
 * 	@param m	The minute.
 * 	@param s	The second. (optional)
 * 	@param a	AM/PM
 * 	@return		The formatted string.
 */
String timeStrBuilder(String h, String m, String s, String a) {
	String str = h;
	str += ":";
	str += m;
	str += ":";
	str += s;
	str += " ";
	str += a;
	return str;
}

/* See above */
String timeStrBuilder(String h, String m, String a) {
	String str = h;
	str += ":";
	str += m;
	str += " ";
	str += a;
	return str;
}
/*
 * 	Utility function for building up a formatted date
 * 	string.  Exists because doing string concatenation
 * 	with the String object is ugly as hell.
 *
 * 	@param w	The week day.
 * 	@param m	The month.
 * 	@param d	The day.
 * 	@param y	The year.
 * 	@return		The formatted string.
 */
String dateStrBuilder(String w, String m, String d, String y) {
	String str = w;
	str += " ";
	str += m;
	str += " ";
	str += d;
	str += " ";
	str += y;
	return str;
}

/*
 * 	Gets the LCD column where the specified
 * 	field begins.  Used for blink indicator.
 *
 * 	@param f	The field.
 * 	@return		The starting column.
 */
int getColumn(field_type f) {
	int theColumn = 0;
	for (int i = 0; i < N_FIELD_TYPES; i++) {
		if (EDITABLE_FIELDS[i].FieldType == f) {
			theColumn = EDITABLE_FIELDS[i].Column;
			break;
		}
	}
	return theColumn;
}

/*
 * 	Utility function which is useful for
 * 	cycling through an enum, for instance
 * 	field types or view modes.
 *
 * 	@param base	The base item.
 * 	@param last	The last item.
 * 	@param actual	The current item.
 * 	@return		the next item, else base.
 */
int nextEnum(const int base, const int last, const int actual) {
	return actual < last ? actual + 1 : base;
}

/*
 * 	Utility function for incrementing a field value
 * 	within a range between high and low.
 *
 * 	@param value	The fields current value
 * 	@param high		Range max.
 * 	@param low		Range min.
 * 	@return			The next field value
 */
int fieldIncrement(const int value, const int high, const int low) {
	return value < high ? value + 1 : low;
}

/*
 *	This routine was part of the timeAdjustment function,
 *	It's ugly and stupid but until I find a better way
 *	of doing this, I'll hide it at the bottom of the source
 *	file.
 *
 *	@param match	The field to match.
 *	@param range	The fields range.
 *	@param toAdjust	Pointer to time info structure.
 */
void matchAndAdjust(field_data match, time_info * toAdjust) {
	switch (match.FieldType) {
	case HOUR:
		toAdjust->Hour = F_INC(toAdjust->Hour, match.Range.High, match.Range.Low);
		break;
	case MINUTE:
		toAdjust->Minute = F_INC(toAdjust->Minute, match.Range.High,
				match.Range.Low);
		break;
	case AMPM:
		toAdjust->AmPm = toAdjust->AmPm == "AM" ? "PM" : "AM";
		break;
	case WDAY:
		toAdjust->Wday = F_INC(toAdjust->Wday, match.Range.High, match.Range.Low);
		break;
	case DAY:
		toAdjust->Day = F_INC(toAdjust->Day, match.Range.High, match.Range.Low);
		break;
	case MONTH:
		toAdjust->Month = F_INC(toAdjust->Month, match.Range.High,
				match.Range.Low);
		break;
	case YEAR:
		toAdjust->Year = F_INC(toAdjust->Year, match.Range.High, match.Range.Low);
		break;
	case NONE:
		// THERE ECLIPSE ARE YOU HAPPY NOW?
		break;
	}

	verifyAdjustment(toAdjust);
}
/*
 * 	Helper function for verifying changes to system time.
 *
 * 	setTime() as defined in Arduino's Time library accepts
 * 	hours in 0-23 format and has no regard for AM or PM, so
 * 	manually adjusting the hour field is required when the user
 * 	toggles the am/pm field.
 *
 * 	@param toVerify`Pointer to the time info structure.
 */
void verifyAdjustment(time_info * toVerify) {
	if ((toVerify->Hour <= 12) && (toVerify->AmPm == "PM")) {
		toVerify->Hour += 12;
	} else if ((toVerify->Hour >= 12) && (toVerify->AmPm == "AM")) {
		toVerify->Hour -= 12;
	}
}
