/**
 File comune con definzioni generali.
 */

#ifndef __COMMONS_HPP
#define __COMMONS_HPP

#include <boost/any.hpp>
#include <boost/regex.hpp>
#include <codecvt>
#include <iostream>
#include <libxml/tree.h>
#include <locale>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#define cout std::cout
#define cerr std::cerr

#ifdef __APPLE__
#define MAC_OS_X 1
#endif

using String = std::string;
using StringVector = std::vector<String>;
using StringMap = std::unordered_map<String, String>;

using Any = boost::any;
using AnyVector = std::vector<Any>;
using AnyStringMap = std::unordered_map<String, Any>;

using IntVector = std::vector<int>;
using IntPair = std::pair<int, int>;

String format(String format, ...);
bool strStartWith(const String &a, const String &b);
bool strEndWith(const String &a, const String &b);
bool isEqu(const String &ptest, const String &p1);
String formatDateIso(time_t t = 0);
String formatDateTimeIso(time_t t = 0);
String formatDate(time_t t = 0);
String formatDateTime(time_t t = 0);
String trim(const char *string);
String trim(const String &s);
String join(const StringVector &vect, const String &separator,
            const String &delimiter);
String joinCommand(const StringVector &vect, const String &separator = " ",
                   const String &delimiter = "\"");
String itoa(int i);
String mergeUri(String protocol, String host, int port, String path);
String readConsole(FILE *console, String prompt, String defVal = "");

int tsCompare(struct timespec time1, struct timespec time2);
int spawn(const String &commandPath, const StringVector &commandLine);
int split(const String &s, StringVector &v);
int split(const String &input, boost::regex &expr, StringVector &v);
int split(const String &input, boost::regex &expr, IntVector &v);
int split(const String &input, const String &expr, StringVector &v);
int split(const String &input, const String &expr, IntVector &v);
int splitComma(const String &input, StringVector &v);
int splitComma(const String &input, IntVector &v);

bool is_empty(const Any &operand);
bool is_int(const Any &operand);
bool is_time(const Any &operand);
bool is_timeval(const Any &operand);
bool is_char_ptr(const Any &operand);
bool is_string(const Any &operand);

inline int anyCastInt(Any value) { return boost::any_cast<int>(value); }
inline String anyCastString(Any value) {
  return boost::any_cast<String>(value);
}

int vector2Properties(AnyStringMap &rv, const StringVector &vstr);
AnyStringMap vector2Properties(const StringVector &vstr);
int countMatchInRegex(String s, String expr);
IntPair parseDays(AnyStringMap &prop);
IntPair parseDays(const StringVector &args, int index);
String toString(const AnyStringMap &properties, const String &separator = ",");
bool contains(String toSearch, const StringVector &names);

#ifndef linux
#define strdupa(x) (strcpy((char *)alloca((strlen(x) + 1) * sizeof(char)), (x)))
#endif

#define TS ("[" + formatDateTime(0) + "] ")

class GenericException : public std::exception {
public:
  inline GenericException(const String &cause) {
    this->cause = cause;
    this->errorStore = errno;
  }

  inline GenericException(const String &cause, int errore) {
    this->cause = cause;
    this->errorStore = errore;
  }

  virtual ~GenericException() throw() {}

  inline virtual const char *what() const throw() { return cause.c_str(); }

  String cause;
  int errorStore;
};

#endif
