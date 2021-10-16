#include "datetime.h"

/// Turn the ASCII representation of an integer into an actual
/// integer.
#define ASCII_INT(x) (x - '0')

/// Extract the year from a date-time string.
static int date_time_year(const char *dt) {
  return 2000
    + ASCII_INT(dt[6]) * 10
    + ASCII_INT(dt[7]);
}

/// Extract the month from a date-time string.
static int date_time_month(const char *dt) {
  return ASCII_INT(dt[3]) * 10
    + ASCII_INT(dt[4]);
}

/// Extract the day from a date-time string.
static int date_time_day(const char *dt) {
  return ASCII_INT(dt[0]) * 10
    + ASCII_INT(dt[1]);
}

/// Extract the hour from a date-time string.
static int date_time_hour(const char *dt) {
  return ASCII_INT(dt[9]) * 10
    + ASCII_INT(dt[10]);
}

/// Extract the minute from a date-time string.
static int date_time_minute(const char *dt) {
  return ASCII_INT(dt[12]) * 10
    + ASCII_INT(dt[13]);
}

/// Extract the second from a date-time string.
static int date_time_second(const char *dt) {
  return ASCII_INT(dt[15]) * 10
    + ASCII_INT(dt[16]);
}

#undef ASCII_INT

/// Compare two date-time strings.
///
/// Returns 0 if `a` and `b` are equal, -1 if `a` is less than `b`,
/// and 1 if `a` is greater than `b`.
int compare_date_time(const char *a, const char *b, int ignore_date) {
#define MY_COMPARE(func) do {						\
    int a_val = func(a);						\
    int b_val = func(b);						\
    if (a_val > b_val) {						\
      return 1;								\
    } else if (b_val > a_val ) {					\
      return -1;							\
    }									\
  } while (0)

  if (ignore_date != 1) {
    MY_COMPARE(date_time_year);
    MY_COMPARE(date_time_month);
    MY_COMPARE(date_time_day);
  }
  MY_COMPARE(date_time_hour);
  MY_COMPARE(date_time_minute);
  MY_COMPARE(date_time_second);
#undef MY_COMPARE

  return 0;
}

void add_date_time(const char *a, char *b, int n) {
  int year = date_time_year(a),
    month = date_time_month(a),
    day = date_time_day(a),
    hour = date_time_hour(a),
    minute = date_time_minute(a),
    second = date_time_second(a);

  second += n; // 61

  minute += second / 60; // +1
  second %= 60; //1

  hour += minute / 60; // 61
  minute %= 60;

  // Incorrect: can go past e.g. 31 days.
  day += hour / 24;
  hour %= 60;

  // Heater doesn't operate close to midnight, so it's okay that this
  // code doesn't work properly with days, or at all with months and
  // years.

  b[0] = ((day / 10) % 10) + '0';
  b[1] = (day % 10) + '0';
  b[2] = '/';
  b[3] = ((month / 10) % 10) + '0';
  b[4] = (month % 10) + '0';
  b[5] = '/';
  b[6] = ((year / 10) % 10) + '0';
  b[7] = (year % 10) + '0';
  b[8] = ' ';
  b[9] = ((hour / 10) % 10) + '0';
  b[10] = (hour % 10) + '0';
  b[11] = ':';
  b[12] = ((minute / 10) % 10) + '0';
  b[13] = (minute % 10) + '0';
  b[14] = ':';
  b[15] = ((second / 10) % 10) + '0';
  b[16] = (second % 10) + '0';
  b[17] = '\0';
}
