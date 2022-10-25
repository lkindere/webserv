#include <map>

#include "Types.hpp"

using namespace std;

/**
 * @brief Returns status string based on error code
 * @param error
 * @return string
 */
string getStatus(short error) {
    map< short, string > status;
    status.insert(make_pair(301, "301 Moved Permanently"));
    status.insert(make_pair(400, "400 Bad Request"));
    status.insert(make_pair(403, "403 Forbidden"));
    status.insert(make_pair(404, "404 Not Found"));
    status.insert(make_pair(405, "405 Method Not Allowed"));
    status.insert(make_pair(411, "411 Length Required"));
    status.insert(make_pair(413, "413 Payload Too Large"));
    status.insert(make_pair(414, "414 URI Too Long"));
    status.insert(make_pair(415, "415 Unsupported Media Type"));
    status.insert(make_pair(500, "500 Internal Server Error"));
    status.insert(make_pair(505, "505 HTTP Version Not Supported"));
    //Add more later if needed if missing will return empty str
    return status[error];
}
#include <iostream>
/**
 * @brief An abomination, give it a .extension, it will spit out a type.
 * @brief Don't ask me how it works, maybe it doesn't, it matters not.
 * @brief The sheer amount of time took copy pasta'ing this is enough to use it.
 * @param extension .something str
 * @return string type str
 */
static string howDoMimesWork(const string& extension) {
    map< string, string > types;
    types.insert(make_pair(".aac", "audio/aac"));
    types.insert(make_pair(".abw", "application/x-abiword"));
    types.insert(make_pair(".arc", "application/x-freearc"));
    types.insert(make_pair(".avi", "video/x-msvideo"));
    types.insert(make_pair(".azw", "application/vnd.amazon.ebook"));
    types.insert(make_pair(".bin", "application/octet-stream"));
    types.insert(make_pair(".bmp", "image/bmp"));
    types.insert(make_pair(".bz", "application/x-bzip"));
    types.insert(make_pair(".bz2", "application/x-bzip2"));
    types.insert(make_pair(".csh", "application/x-csh"));
    types.insert(make_pair(".css", "text/css"));
    types.insert(make_pair(".csv", "text/csv"));
    types.insert(make_pair(".doc", "application/msword"));
    types.insert(make_pair(".epub", "application/epub+zip"));
    types.insert(make_pair(".gif", "image/gif"));
    types.insert(make_pair(".htm", "text/html"));
    types.insert(make_pair(".html", "text/html"));
    types.insert(make_pair(".ico", "image/x-icon"));
    types.insert(make_pair(".ics", "text/calendar"));
    types.insert(make_pair(".jar", "application/java-archive"));
    types.insert(make_pair(".jpeg", "image/jpeg"));
    types.insert(make_pair(".jpg", "image/jpeg"));
    types.insert(make_pair(".js", "application/js"));
    types.insert(make_pair(".json", "application/json"));
    types.insert(make_pair(".mid", "audio/midi"));
    types.insert(make_pair(".midi", "audio/midi"));
    types.insert(make_pair(".mpeg", "video/mpeg"));
    types.insert(make_pair(".mpkg", "application/vnd.apple.installer+xml"));
    types.insert(make_pair(".odp", "application/vnd.oasis.opendocument.presentation"));
    types.insert(make_pair(".ods", "application/vnd.oasis.opendocument.spreadsheet"));
    types.insert(make_pair(".odt", "application/vnd.oasis.opendocument.text"));
    types.insert(make_pair(".oga", "audio/ogg"));
    types.insert(make_pair(".ogv", "video/ogg"));
    types.insert(make_pair(".ogx", "application/ogg"));
    types.insert(make_pair(".pdf", "application/pdf"));
    types.insert(make_pair(".ppt", "application/vnd.ms-powerpoint"));
    types.insert(make_pair(".rar", "application/x-rar-compressed"));
    types.insert(make_pair(".rtf", "application/rtf"));
    types.insert(make_pair(".sh", "application/x-sh"));
    types.insert(make_pair(".svg", "image/svg+xml"));
    types.insert(make_pair(".swf", "application/x-shockwave-flash"));
    types.insert(make_pair(".tar", "application/x-tar"));
    types.insert(make_pair(".tif", "image/tiff"));
    types.insert(make_pair(".tiff", "image/tiff"));
    types.insert(make_pair(".ttf", "application/x-font-ttf"));
    types.insert(make_pair(".vsd", "application/vnd.visio"));
    types.insert(make_pair(".wav", "audio/x-wav"));
    types.insert(make_pair(".weba", "audio/webm"));
    types.insert(make_pair(".webm", "video/webm"));
    types.insert(make_pair(".webp", "image/webp"));
    types.insert(make_pair(".woff", "application/x-font-woff"));
    types.insert(make_pair(".xhtml", "application/xhtml+xml"));
    types.insert(make_pair(".xls", "application/vnd.ms-excel"));
    types.insert(make_pair(".xml", "application/xml"));
    types.insert(make_pair(".xul", "application/vnd.mozilla.xul+xml"));
    types.insert(make_pair(".zip", "application/zip"));
    types.insert(make_pair(".3gp", "video/3gpp"));
    types.insert(make_pair(".3g2", "video/3gpp2"));
    types.insert(make_pair(".7z", "application/x-7z-compressed"));
    map< string, string >::const_iterator it(types.find(extension));
    if (it != types.end()){
        cout << "Type: " << it->second;
        return it->second;
    }
    return "text/plain";
}

/**
 * @brief Gets file type from path
 * @param path
 * @return string
 */
string getType(const string& path){
    size_t i = path.find('.');
    if (i == path.npos)
        return "text/plain";
    return howDoMimesWork(path.substr(i));
}
