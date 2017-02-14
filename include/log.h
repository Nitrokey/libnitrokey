#ifndef LOG_H
#define LOG_H
#include <string>
#include <cstddef>

namespace nitrokey {
namespace log {

#ifdef ERROR
#undef ERROR
#endif

enum class Loglevel : int { DEBUG_L2, DEBUG, INFO, WARNING, ERROR };

class LogHandler {
 public:
  virtual void print(const std::string &, Loglevel lvl) = 0;

 protected:
  std::string loglevel_to_str(Loglevel);
};

class StdlogHandler : public LogHandler {
 public:
  virtual void print(const std::string &, Loglevel lvl);
};

extern StdlogHandler stdlog_handler;

class Log {
 public:
  Log() : mp_loghandler(&stdlog_handler), m_loglevel(Loglevel::WARNING) {}

  static Log &instance() {
    if (mp_instance == NULL) mp_instance = new Log;
    return *mp_instance;
  }

  void operator()(const std::string &, Loglevel);

  void set_loglevel(Loglevel lvl) { m_loglevel = lvl; }

  void set_handler(LogHandler *handler) { mp_loghandler = handler; }

 private:
  Loglevel m_loglevel;
  LogHandler *mp_loghandler;

  static Log *mp_instance;
};
}
}

#endif
