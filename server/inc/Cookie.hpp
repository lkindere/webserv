#pragma once

#include "CookieDate.hpp"

#include <sstream>
#include <string>


class Cookie {

    /* Member Types */
public:
    enum sameSiteEnum {
        Strict,
        Lax,
        None,
        Empty
    };

private:
    static const int _MaxAgeNotSpecified = INT_FAST8_MIN;

    /* Private Members */
private:
    std::string _name;
    std::string _value;
    CookieDate _expires;
    int _maxAge;
    std::string _domain;
    std::string _path;
    bool _httpOnly;
    sameSiteEnum _sameSite;

    /* Constructor */
public:
    Cookie(const std::string &name, const std::string &value);

    /* Public Member Functions */
public:
    std::string toString() const;

    /* Setter / Getter */
public:
    void setName(const std::string &name) { this->_name = name; }
    void setValue(const std::string &value) { this->_value = value; }
    void setExpires(const CookieDate &expires) { this->_expires = expires; }
    void setAge(int maxAge) { this->_maxAge = maxAge; }
    void setDomain(const std::string &domain) { this->_domain = domain; }
    void setPath(const std::string &path) { this->_path = path; }
    void setHttpOnly(bool httpOnly) { this->_httpOnly = httpOnly; }
    void setSameSite(sameSiteEnum sameSite) { this->_sameSite = sameSite; }

    std::string getName() { return this->_name; }
    std::string getValue() { return this->_value; }
    CookieDate getExpires() { return this->_expires; }
    int getMaxAge() { return this->_maxAge; }
    std::string getDomain() { return this->_domain; }
    std::string getPath() { return this->_path; }
    bool getHttpOnly() { return this->_httpOnly; }
    sameSiteEnum getSameSite() { return this->_sameSite; }

private:
    std::string _sameSiteEnumToString(sameSiteEnum sameSite) const;
	bool _isNonSpecifiedCookieDate(const CookieDate &cookieDate) const;
};