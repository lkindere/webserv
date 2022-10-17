#include "Cookie.hpp"

Cookie::Cookie(const std::string &name,
               const std::string &value)
    : _name(name),
      _value(value),
      _expires(NonSpecifiedCookieDate()),
      _maxAge(_MaxAgeNotSpecified),
      _domain(""),
      _path(""),
      _httpOnly(false),
      _sameSite(Empty) {}

std::string Cookie::toString() const {
    static const std::string quote = "\"";
    static const std::string delimiter = "; ";

    std::stringstream ss_result;

    ss_result << this->_name
              << "=" << quote << this->_value << quote;

    if (!this->_isNonSpecifiedCookieDate(this->_expires)) {
        ss_result << delimiter << "Expires=" << quote
                  << this->_expires.toString() << quote;
    }

    if (this->_maxAge != _MaxAgeNotSpecified) {
        ss_result << delimiter << "MaxAge=" << quote
                  << this->_maxAge << quote;
    }

    if (!this->_domain.empty()) {
        ss_result << delimiter << "Domain=" << quote
                  << this->_domain << quote;
    }

    if (!this->_path.empty()) {
        ss_result << delimiter << "Path=" << quote
                  << this->_path << quote;
    }

    if (this->_httpOnly) {
        ss_result << delimiter << "HttpOnly";
    }

    if (this->_sameSite != Empty) {
        ss_result << delimiter << "SameSite=" << quote
                  << this->_sameSiteEnumToString(this->_sameSite) << quote;
    }

    return ss_result.str();
}

bool Cookie::_isNonSpecifiedCookieDate(const CookieDate &cookieDate) const {

    try {
        const CookieDate result = dynamic_cast< const NonSpecifiedCookieDate & >(cookieDate);
    } catch (...) {
        return true;
    }
    return false;
}

std::string Cookie::_sameSiteEnumToString(sameSiteEnum sameSite) const {
    static const std::string enumAsString[] = {"Strict", "Lax", "None"};

    if (sameSite < Strict || sameSite > None)
        return std::string();

    return enumAsString[sameSite];
}