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
	setTime (
				seed.Hour  , seed.Minute,
				seed.Second, seed.Day,
				seed.Month , seed.Year
	);

	alarm->Hour = 12;
	alarm->Minute = 30;
	alarm->AmPm = "PM";

	*set = *alarm;

	set->Wday = weekday( now() );
	set->Day = seed.Day;
	set->Month = seed.Month;
	set->Year = seed.Year;
}

/*
 * 	Renders the default alarm clock view
 *
 * 	@param lcd			LCD Object.
 * 	@param to_render	A time.
 */
void renderDefault(LiquidCrystal lcd, time_t to_render) {

	lcd.setCursor( 0 , 0 );
	lcd.print( dateFormatStr( to_render ) );

	lcd.setCursor( 0 , 1);
	lcd.print( timeFormatStr( to_render ) );
}

/*
 * 	Renders the time set view
 *
 * 	@param lcd			LCD Object.
 * 	@param selected 	Currently selected field.
 * 	@param to_render	A time.
 */
void renderTimeset(LiquidCrystal lcd, time_info to_render, field_type selected) {

	lcd.setCursor( 0 , 0 );
	lcd.print("Set clock time");

	lcd.setCursor( 0 , 1 );
	lcd.print( timeInfoFormatStr ( to_render ) );

	if ( selected != NONE ) {
		lcd.setCursor( getColumn( selected ), 1 );
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
void renderDateset(LiquidCrystal lcd, time_info to_render, field_type selected) {

	lcd.setCursor( 0 , 0 );
	lcd.print("Set clock date");

	lcd.setCursor( 0 , 1 );
	lcd.print( dateInfoFormatStr ( to_render ) );

	if ( selected != NONE ) {
		lcd.setCursor( getColumn( selected ), 1 );
		lcd.blink();
	}
}

/*
 * 	Renders the alarm set view
 *
 * 	@param lcd			LCD Object.
 * 	@param to_render	Alarm time.
 */
void renderAlarmset(LiquidCrystal lcd, time_info to_render, field_type selected) {
	lcd.setCursor( 0 , 0 );
	lcd.print("Set alarm time");

	lcd.setCursor( 0 , 1 );
	lcd.print( timeInfoFormatStr( to_render ) );

	if ( selected != NONE ) {
		lcd.setCursor( getColumn( selected ), 1 );
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
	delay( 125 );
	adjustTime( SYS_TIME_ADJUSTMENT );
}
/*
 * Increments a date/time field value stored
 * in a time info structure.
 *
 * @param field	The field to increment.
 * @param t		Time info pointer.
 */
void timeInfoIncrement( field_type field , time_info *t) {
	switch ( field ) {
		case HOUR:
			t->Hour = t->Hour >= 24 ? 1 : t->Hour + 1;
			break;
		case MINUTE:
			t->Minute = t->Minute >= 59 ? 1 : t->Minute + 1;
			break;
		case AMPM:
			t->AmPm = t->AmPm == "PM" ? "AM" : "PM";
			break;
		case WDAY:
			t->Wday = t->Wday < 7 ? t->Wday + 1 : 1;
			break;
		case DAY:
			t->Day = t->Day < 31 ? t->Day + 1 : 1;
			break;
		case MONTH:
			t->Month = t->Month < 12 ? t->Month + 1 : 1;
			break;
		case YEAR:
			t->Year = t->Year < 2020 ? t->Year + 1 : 2015;
			break;
		default:
			break;

	}
}
/*
 * 	Sets the system time based on values
 * 	keyed in by the user.
 *
 * 	@param set	A time.
 */
void setClockTime(time_info set) {
	setTime (
				set.Hour  , set.Minute,
				set.Second, set.Day,
				set.Month , set.Year
	);
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

	if ( digitalRead( ADJUST_SWITCH ) ) {
		which = ADJUST_SWITCH;
	} else if ( digitalRead( SELECT_SWITCH ) ) {
		which = SELECT_SWITCH;
	} else if ( digitalRead( PAGE_SWITCH ) ) {
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
	if ( current == V_TIMESET  ||
		 current == V_ALARMSET ||
		 current == V_DATESET ) {
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
	view_mode next;
	if ( current == V_DEFAULT ) {
		next = V_TIMESET;
	} else if ( current == V_TIMESET ) {
		next = V_DATESET;
	} else if ( current == V_DATESET ) {
		next = V_ALARMSET;
	} else {
		next = V_DEFAULT;
	}

	return next;
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
field_type	nextField(view_mode current, field_type selected) {
	field_type next = NONE;
	switch ( current ) {
		case V_TIMESET:
			if ( selected == NONE ) {
				next = HOUR;
			} else if ( selected == HOUR ) {
				next = MINUTE;
			} else if ( selected == MINUTE ) {
				next = AMPM;
			} else if ( selected == AMPM ){
				next = HOUR;
			}
			break;
		case V_ALARMSET:
			if ( selected == NONE ) {
				next = HOUR;
			} else if ( selected == HOUR ) {
				next = MINUTE;
			} else if ( selected == MINUTE ) {
				next = AMPM;
			} else if ( selected == AMPM ) {
				next = HOUR;
			}
			break;
		case V_DATESET:
			if ( selected == NONE ) {
				next = WDAY;
			} else if ( selected == WDAY ) {
				next = MONTH;
			} else if ( selected == MONTH ) {
				next = DAY;
			} else if ( selected == DAY ) {
				next = YEAR;
			} else if ( selected == YEAR ) {
				next = WDAY;
			}
			break;
		default:
			break;
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
int calcTemp(int sensorValue) {
	float volts   = ( sensorValue / 1024.0 ) * 5.0;
	float celsius = ( volts - 0.5 ) * 100;
	return trunc( celsius * 1.8 ) + 32;
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
	String weekDay  = String( STR_WEEKDAY[weekday(now)] );
	String monthStr = String( STR_MONTH[month(now)] );
	int iDay  = day( now );
	int iYear = year( now );

	String dayFrmt  = "";
	iDay < 10 ? dayFrmt.concat('0') : 0;
	dayFrmt.concat(iDay);

	String yearFrmt = String( iYear );

	String ret = weekDay;
	ret.concat(" ");
	ret.concat( monthStr );
	ret.concat(" ");
	ret.concat( dayFrmt );
	ret.concat(" ");
	ret.concat( yearFrmt );
	return ret;
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

	int hr  = hourFormat12( now );
	int min = minute( now );
	int sec = second( now );

	String hrFrmt = "";
	hr < 10 ? hrFrmt.concat('0') : 0;
	hrFrmt.concat(hr);

	String minFrmt = "";
	min < 10 ? minFrmt.concat('0') : 0;
	minFrmt.concat( min );

	String secFrmt = "";
	sec < 10 ? secFrmt.concat('0') : 0;
	secFrmt.concat( sec );

	String ampm = isAM( now ) ? "AM" : "PM";

	String ret = hrFrmt;
	ret.concat(":");
	ret.concat( minFrmt );
	ret.concat(":");
	ret.concat( secFrmt );
	ret.concat(" ");
	ret.concat( ampm );
	return ret;
}

String dateInfoFormatStr(time_info t) {
	String weekDay  = String( STR_WEEKDAY[t.Wday] );
	String monthStr = String( STR_MONTH[t.Month] );

	String dayFrmt  = "";
	t.Day < 10 ? dayFrmt.concat('0') : 0;
	dayFrmt.concat(t.Day);

	String yearFrmt = String( t.Year );

	String ret = weekDay;
	ret.concat(" ");
	ret.concat( monthStr );
	ret.concat(" ");
	ret.concat( dayFrmt );
	ret.concat(" ");
	ret.concat( yearFrmt );
	return ret;
}
String timeInfoFormatStr(time_info t) {

	int hr  = t.Hour > 12 ? t.Hour - 12 : t.Hour;
	int min = t.Minute;

	String hrFrmt = "";
	hr < 10 ? hrFrmt.concat('0') : 0;
	hrFrmt.concat(hr);

	String minFrmt = "";
	min < 10 ? minFrmt.concat('0') : 0;
	minFrmt.concat( min );

	String ampm = t.AmPm;

	String ret = hrFrmt;
	ret.concat(":");
	ret.concat( minFrmt );
	ret.concat(" ");
	ret.concat( ampm );
	return ret;
}
/*
 * 	Utility function for converting a time_t
 * 	into a time_info structure.
 *
 * 	@param t	A time.
 * 	@return		A time_info structure.
 */
time_info timeInfo(time_t t) {
	time_info theInfo;
	theInfo.Second = second(t);
	theInfo.Minute = minute(t);
	theInfo.Hour = hour(t);
	theInfo.Wday = weekday(t);
	theInfo.Day = day(t);
	theInfo.Year = year(t);
	return theInfo;
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

	switch ( f ) {
		case WDAY:
			theColumn = WDAY_COL;
			break;
		case MONTH:
			theColumn = MONTH_COL;
			break;
		case DAY:
			theColumn = DAY_COL;
			break;
		case YEAR:
			theColumn = YEAR_COL;
			break;
		case HOUR:
			theColumn = HOUR_COL;
			break;
		case MINUTE:
			theColumn = MINUTE_COL;
			break;
		case AMPM:
			theColumn = AMPM_COL;
			break;
		default:
			break;
	}

	return theColumn;
}
