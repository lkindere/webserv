#pragma once

#include <exception>
#include <string>

struct CookieDate {

private:
    std::string _dayName;
    std::string _day;
    std::string _month;
    std::string _year;
    std::string _hour;
    std::string _minute;
    std::string _second;

protected:
    CookieDate() /* = delete */;

public:
    // Throws on Failure
    CookieDate(const std::string &dayName, const std::string &day, const std::string &month, const std::string &year, const std::string &hour, const std::string &minute, const std::string &second);
    CookieDate(const CookieDate &cookieDate);

    virtual ~CookieDate() {}

    /* Private Member Functions */
private:
    int _getNumberOfDaysFromMonthAndYear(const int month, const int year) const;
    bool _isValidDayName(const std::string &dayName) const;
    bool _isValidDay(const std::string &day) const;
    bool _isValidMonth(const std::string &month) const;
    bool _isValidYear(const std::string &year) const;
    bool _isValidHour(const std::string &hour) const;
    bool _isValidMinute(const std::string &minute) const;
    bool _isValidSecond(const std::string &second) const;

    /* Public Member Functions */
public:
    std::string toString() const;

    /* Setter / Getter */
public:
    void setDayName(const std::string &dayName);
    void setDay(const std::string &day);
    void setMonth(const std::string &month);
    void setYear(const std::string &year);
    void setHour(const std::string &hour);
    void setMinute(const std::string &minute);
    void setSecond(const std::string &second);

    std::string getDayName() const { return this->_dayName; }
    std::string getDay() const { return this->_day; }
    std::string getMonth() const { return this->_month; }
    int getMonthAsNumber() const;
    std::string getYear() const { return this->_year; }
    std::string getHour() const { return this->_hour; }
    std::string getMinut() const { return this->_minute; }
    std::string getSecond() const { return this->_second; }
};
