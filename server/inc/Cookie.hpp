#pragma once

#include "CookieDate.hpp"

#include <sstream>
#include <string>
#include <stdint.h>


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
    CookieDate *_expires;
    int _maxAge;
    std::string _domain;
    std::string _path;
    bool _httpOnly;
    sameSiteEnum _sameSite;

    /* Constructor */
public:
    Cookie(const std::string &name, const std::string &value);

    ~Cookie() { delete this->_expires; }
    /* Public Member Functions */
public:
    std::string toString() const;

    /* Setter / Getter */
public:
    void setName(const std::string &name);
    void setValue(const std::string &value);
    void setExpires(const CookieDate &expires);
    void setAge(int maxAge);
    void setDomain(const std::string &domain);
    void setPath(const std::string &path);
    void setHttpOnly(bool httpOnly);
    void setSameSite(sameSiteEnum sameSite);

    std::string getName() const;
    std::string getValue() const;
    CookieDate getExpires() const;
    int getMaxAge() const;
    std::string getDomain() const;
    std::string getPath() const;
    bool getHttpOnly() const;
    sameSiteEnum getSameSite() const;

private:
    std::string _sameSiteEnumToString(sameSiteEnum sameSite) const;
};