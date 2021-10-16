#ifndef DATETIME_H
#define DATETIME_H

/// Compare two date-time strings.
///
/// Returns 0 if `a` and `b` are equal, -1 if `a` is less than `b`,
/// and 1 if `a` is greater than `b`.  Ignores date if `ignore_date` is 1.
int compare_date_time(const char *a, const char *b, int ignore_date);

/// Add to a date-time string.
///
/// Adds `n` seconds to the date-time string in `a` and stores it in
/// `b`.
void add_date_time(const char *a, char *b, int n);

#endif