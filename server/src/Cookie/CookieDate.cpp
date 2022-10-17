#include "CookieDate.hpp"

#include <iostream>
#include <exception>

#include "uString.hpp"

CookieDate::CookieDate() {
	// throw std::logic_error("Default Constructor should not be called");
}

// Throws on Failure
CookieDate::CookieDate(const std::string &dayName, const std::string &day, const std::string &month, const std::string &year, const std::string &hour, const std::string &minute, const std::string &second) {
    try {
        this->setDayName(dayName);
        this->setYear(year);
        this->setMonth(month);
        this->setDay(day);
        this->setHour(hour);
        this->setMinute(minute);
        this->setSecond(second);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        throw;
    }
}

CookieDate::CookieDate(const CookieDate &cookieDate) {
    try {
        this->setDayName(cookieDate.getDayName());
        this->setYear(cookieDate.getYear());
        this->setMonth(cookieDate.getMonth());
        this->setDay(cookieDate.getDay());
        this->setHour(cookieDate.getHour());
        this->setMinute(cookieDate.getMinut());
        this->setSecond(cookieDate.getSecond());
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        throw;
    }
}

/* Public Member Functions */
std::string CookieDate::toString() const {
	return "NotImplemented";
}

/* Private Member Functions */

bool CookieDate::_isValidDayName(const std::string &dayName) const {
    static const std::string validDayNames[] = {
        "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

    bool isDayNameValid = false;
    for (size_t i = 0; i < sizeof(validDayNames) / sizeof(validDayNames[0]); ++i) {
        if (validDayNames[i] == dayName) {
            isDayNameValid = true;
            break;
        }
    }
    return isDayNameValid;
}

int CookieDate::_getNumberOfDaysFromMonthAndYear(const int month, const int year) const {
    static const int maxNumberOfDaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int numberOfDays = int();
    if (month == 2 && ((year % 400 == 0) || ((year % 100 != 0) && (year % 4 == 0))))
        numberOfDays = maxNumberOfDaysInMonth[month - 1] + 1;
    else
        numberOfDays = maxNumberOfDaysInMonth[month - 1];

    return numberOfDays;
}

bool CookieDate::_isValidDay(const std::string &day) const {
    if (day.length() != 2)
        return false;

    const int dayAsInt = FromString< int >(day);
    const int monthAsInt = FromString< int >(this->getMonth());
    const int yearAsInt = FromString< int >(this->getYear());

    const int numberOfDaysInMonthAndYear = this->_getNumberOfDaysFromMonthAndYear(monthAsInt, yearAsInt);

    if (dayAsInt <= 0 || dayAsInt > numberOfDaysInMonthAndYear)
        return false;

    return true;
}

bool CookieDate::_isValidMonth(const std::string &month) const {
    if (month.length() != 3)
        return false;

    static const std::string validMonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    bool isValidMonth = false;
    for (size_t i = 0; i < sizeof(validMonths) / sizeof(validMonths[0]); ++i) {
        if (validMonths[i] == month) {
            isValidMonth = true;
            break;
        }
    }
    return isValidMonth;
}

bool CookieDate::_isValidYear(const std::string &year) const {
    if (year.length() != 4)
        return false;

    return true;
}

bool CookieDate::_isValidHour(const std::string &hour) const {
    if (hour.length() != 2)
        return false;

    const int hourAsInt = FromString< int >(hour);
    if (hourAsInt <= 0 || hourAsInt > 24)
        return false;

    return true;
}

bool CookieDate::_isValidMinute(const std::string &minute) const {
    if (minute.length() != 2)
        return false;

    const int minuteAsInt = FromString< int >(minute);
    if (minuteAsInt < 0 || minuteAsInt >= 59)
        return false;

    return true;
}

bool CookieDate::_isValidSecond(const std::string &second) const {
    if (second.length() != 2)
        return false;

    const int secondAsInt = FromString< int >(second);
    if (secondAsInt < 0 || secondAsInt >= 59)
        return false;

    return true;
}

/* Setter / Getter */
void CookieDate::setDayName(const std::string &dayName) {
    if (!this->_isValidDayName(dayName))
        throw std::invalid_argument("Invalid Argument for dayName");

    this->_dayName = dayName;
}

void CookieDate::setDay(const std::string &day) {
    if (!this->_isValidDay(day))
        throw std::invalid_argument("Invalid Argument for day");

    this->_day = day;
}

void CookieDate::setMonth(const std::string &month) {
    if (!this->_isValidMonth(month))
        throw std::invalid_argument("Invalid Argument for Month");

    this->_month = month;
}

void CookieDate::setYear(const std::string &year) {
    if (!this->_isValidYear(year))
        throw std::invalid_argument("Invalid Argument for Year");

    this->_year = year;
}

void CookieDate::setHour(const std::string &hour) {
    if (!this->_isValidHour(hour))
        throw std::invalid_argument("Invalid Argument for Hour");

    this->_hour = hour;
}

void CookieDate::setMinute(const std::string &minute) {
    if (!this->_isValidMinute(minute))
        throw std::invalid_argument("Invalid Argument for Minute");

    this->_minute = minute;
}

void CookieDate::setSecond(const std::string &second) {
    if (!this->_isValidSecond(second))
        throw std::invalid_argument("Invalid Argument for Second");

    this->_second = second;
}
