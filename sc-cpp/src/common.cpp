#include "common.hpp"
#include <bits/types/struct_timeval.h>
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

String format(String fmt, ...) {
  va_list va;

  va_start(va, fmt);
  int nbytes = vsnprintf(0, 0, fmt.c_str(), va);
  va_end(va);

  char *buf = (char *)alloca(nbytes + 31);

  va_start(va, fmt);
  vsnprintf(buf, nbytes + 30, fmt.c_str(), va);
  va_end(va);

  return buf;
}

bool strStartWith(const String &a, const String &b) {
  int la = a.length(), lb = b.length();
  return la < lb ? 0 : strncmp(a.c_str(), b.c_str(), lb) == 0;
}

bool strEndWith(const String &a, const String &b) {
  int la = a.length(), lb = b.length();
  return la < lb ? 0 : strncmp(a.c_str() + la - lb, b.c_str(), lb) == 0;
}

bool isEqu(const String &ptest, const String &p1) {
  String test = trim(ptest.c_str());
  String s1 = trim(p1.c_str());
  if (test == s1)
    return true;

  return false;
}

String formatDateIso(time_t t /*= 0*/) {
  if (t == 0)
    time(&t);

  tm tm;
  localtime_r(&t, &tm);

  char buf[128];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1,
           tm.tm_mday);

  return buf;
}

String formatDateTimeIso(time_t t /*= 0*/) {
  if (t == 0)
    time(&t);

  tm tm;
  localtime_r(&t, &tm);

  char buf[128];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900,
           tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

  return buf;
}

String formatDate(time_t t /*= 0*/) {
  if (t == 0)
    time(&t);

  tm tm;
  localtime_r(&t, &tm);

  char buf[128];
  snprintf(buf, sizeof(buf), "%02d/%02d/%04d", tm.tm_mday, tm.tm_mon + 1,
           tm.tm_year + 1900);

  return buf;
}

String formatDateTime(time_t t /*= 0*/) {
  if (t == 0)
    time(&t);

  tm tm;
  localtime_r(&t, &tm);

  char buf[128];
  snprintf(buf, sizeof(buf), "%02d/%02d/%04d %02d:%02d:%02d", tm.tm_mday,
           tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

  return buf;
}

String trim(const char *__string) {
  char *pc1, *pc2, *string = strdupa(__string);

  // Find in pc1 the first non space character
  // if not found, string is empty
  for (pc1 = string; (*pc1 != '\0') && (isspace(*pc1)); pc1++)
    ;
  if (*pc1 == '\0')
    return "";

  // Ship trailing spaces
  for (pc2 = string + strlen(string) - 1; isspace(*pc2) && pc2 > pc1; pc2--)
    ;
  pc2++;
  *pc2 = '\0';

  return pc1;
}

String trim(const String &s) { return trim(s.c_str()); }

int tsCompare(struct timespec time1, struct timespec time2) {
  if (time1.tv_sec < time2.tv_sec)
    return (-1); /* Less than. */
  else if (time1.tv_sec > time2.tv_sec)
    return (1); /* Greater than. */
  else if (time1.tv_nsec < time2.tv_nsec)
    return (-1); /* Less than. */
  else if (time1.tv_nsec > time2.tv_nsec)
    return (1); /* Greater than. */
  else
    return (0); /* Equal. */
}

String join(const StringVector &vect, const String &separator,
            const String &delimiter) {
  String rv;
  rv.reserve(4096);

  int count = 0;
  for (auto s : vect) {
    if (count)
      rv += separator;

    rv += delimiter;
    rv += s;
    rv += delimiter;

    count++;
  }

  return rv;
}

String joinCommand(const StringVector &vect, const String &separator /*= " "*/,
                   const String &delimiter /*= "\""*/) {
  String rv;
  rv.reserve(4096);

  int count = 0;
  for (auto s : vect) {
    if (count)
      rv += separator;

    if (s.find(" ") != String::npos) {
      rv += delimiter;
      rv += s;
      rv += delimiter;
    } else
      rv += s;

    count++;
  }

  return rv;
}

int spawn(const String &commandPath, const StringVector &commandLine) {
  int pid;

  switch (pid = fork()) {
  case -1:
    cout << "FATAL: risorse di sistema esaurite (fork).\n";
    return -1;

  case 0:
    // processo figlio: avvia programma esterno
    break;

  default:
    // processo padre: ritorna pid processo figlio al chiamante
    return pid;
  }

  int num = commandLine.size();
  char *argv[num + 1];
  for (int i = 0; i < num; i++)
    argv[i] = strdupa(commandLine[i].c_str());
  argv[num] = NULL;

  execv(commandPath.c_str(), argv);
  cout << "FATAL: mancato avvio programma " << commandPath << " errore "
       << strerror(errno) << "\n";
  exit(1);
}

int split(const String &input, StringVector &v) {
  // boost::regex expr("\\w+");
  boost::regex expr("[^\\s]+");
  return split(input, expr, v);
}

int split(const String &input, boost::regex &expr, StringVector &v) {
  int count = 0;
  // copia necessaria: il modificatore const non piace
  String s = input;
  boost::regex_token_iterator<String::iterator> it(s.begin(), s.end(), expr);
  boost::regex_token_iterator<String::iterator> end;
  while (it != end) {
    String tmp(trim(*it++));
    if (!tmp.empty())
      v.push_back(tmp);
    count++;
  }
  return count;
}

int split(const String &input, boost::regex &expr, IntVector &v) {
  int count = 0;
  // copia necessaria: il modificatore const non piace
  String s = input;
  boost::regex_token_iterator<String::iterator> it(s.begin(), s.end(), expr);
  boost::regex_token_iterator<String::iterator> end;
  while (it != end) {
    String tmp(trim(*it++));
    if (!tmp.empty())
      v.push_back(atoi(tmp.c_str()));
    count++;
  }
  return count;
}

int splitComma(const String &input, StringVector &v) {
  boost::regex expr("[^,]+");
  return split(input, expr, v);
}

int splitComma(const String &input, IntVector &v) {
  boost::regex expr("[^,]+");
  return split(input, expr, v);
}

String itoa(int i) {
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "%d", i);
  return buffer;
}

String mergeUri(String protocol, String host, int port, String path) {
  if (protocol.empty())
    protocol = "http";
  if (path[0] != '/')
    path = "/" + path;

  String rv = protocol + "://" + host;
  if (port != 0)
    rv += ":" + itoa(port);

  return rv + path;
}

String readConsole(FILE *console, String prompt, String defVal /*= ""*/) {
  char *s, buffer[512];

  cout << prompt;
  fflush(stdout);
  if ((s = fgets(buffer, sizeof(buffer), console)) == NULL)
    return "EOF";

  String rv(trim(s));
  return rv.empty() ? defVal : rv;
}

bool is_empty(const Any &operand) { return operand.empty(); }

bool is_int(const Any &operand) { return operand.type() == typeid(int); }

bool is_time(const Any &operand) { return operand.type() == typeid(time_t); }
bool is_timeval(const Any &operand) {
  return operand.type() == typeid(timeval);
}

bool is_char_ptr(const Any &operand) {
  try {
    boost::any_cast<const char *>(operand);
    return true;
  } catch (const boost::bad_any_cast &) {
    return false;
  }
}

bool is_string(const Any &operand) {
  return boost::any_cast<std::string>(&operand);
}

int vector2Properties(AnyStringMap &rv, const StringVector &vstr) {
  int count = 0;
  for (auto str : vstr) {
    std::string::size_type found = str.find("=");
    if (found != String::npos) {
      // cout << "**" << str.substr(0, found) << "=" << str.substr(found + 1) <<
      // "\n";
      rv[str.substr(0, found)] = str.substr(found + 1);
      count++;
    }
  }
  return count;
}

AnyStringMap vector2Properties(const StringVector &vstr) {
  AnyStringMap rv;
  vector2Properties(rv, vstr);
  return rv;
}

int countMatchInRegex(String s, String sexpr) {
  boost::regex expr(sexpr);
  auto begin = boost::make_regex_iterator(s, expr);
  return std::distance(begin, {});
}

IntPair parseDays(AnyStringMap &properties) {
  int dayStart = 0;
  int dayStop = 365;
  if (is_int(properties["daystart"])) {
    dayStart = anyCastInt(properties["daystart"]);
    if (dayStart < 0)
      dayStart = 0;
  }
  if (is_int(properties["daystop"])) {
    dayStop = anyCastInt(properties["daystop"]);
    if (dayStop > 365)
      dayStop = 365;
  }

  if (dayStart < 0 || dayStart > 365 || dayStop < dayStart || dayStop > 365)
    throw GenericException("Valori non corretti per daystart/daystop.");

  return std::make_pair(dayStart, dayStop);
}

IntPair parseDays(const StringVector &args, int index) {
  int dayStart = 0, dayStop = 365;
  if (args.size() > index) {
    dayStart = atoi(args[index].c_str());
  }
  index++;
  if (args.size() > index) {
    dayStop = atoi(args[index].c_str());
  }

  if (dayStart < 0 || dayStart > 365 || dayStop < dayStart || dayStop > 365)
    throw GenericException("Valori non corretti per daystart/daystop.");

  return std::make_pair(dayStart, dayStop);
}

String toString(const AnyStringMap &properties,
                const String &separator /* = "," */) {
  String rv;
  rv.reserve(1024);

  for (auto it : properties) {
    if (!rv.empty())
      rv.append(separator);
    auto copy = it.second;
    rv.append(it.first + "=" + boost::any_cast<std::string>(copy));
  }

  return rv;
}
