#include "Cookie.hpp"

Cookie::Cookie(const std::string &name,
               const std::string &value)
    : _name(name),
      _value(value),
      _expires(NULL),
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

    if (this->_expires != NULL) {
        ss_result << delimiter << "Expires=" << quote
                  << this->_expires->toString() << quote;
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

std::string Cookie::_sameSiteEnumToString(sameSiteEnum sameSite) const {
    static const std::string enumAsString[] = {"Strict", "Lax", "None"};

    if (sameSite < Strict || sameSite > None)
        return std::string();

    return enumAsString[sameSite];
}

/* Getter / Setter */

void Cookie::setName(const std::string &name) { this->_name = name; }
void Cookie::setValue(const std::string &value) { this->_value = value; }
void Cookie::setExpires(const CookieDate &expires) {
    if (this->_expires != NULL)
        delete this->_expires;

    this->_expires = new CookieDate(expires);
}
void Cookie::setAge(int maxAge) { this->_maxAge = maxAge; }
void Cookie::setDomain(const std::string &domain) { this->_domain = domain; }
void Cookie::setPath(const std::string &path) { this->_path = path; }
void Cookie::setHttpOnly(bool httpOnly) { this->_httpOnly = httpOnly; }
void Cookie::setSameSite(sameSiteEnum sameSite) { this->_sameSite = sameSite; }

std::string Cookie::getName() const { return this->_name; }
std::string Cookie::getValue() const { return this->_value; }
CookieDate Cookie::getExpires() const { return *this->_expires; }
int Cookie::getMaxAge() const { return this->_maxAge; }
std::string Cookie::getDomain() const { return this->_domain; }
std::string Cookie::getPath() const { return this->_path; }
bool Cookie::getHttpOnly() const { return this->_httpOnly; }
Cookie::sameSiteEnum Cookie::getSameSite() const { return this->_sameSite; }