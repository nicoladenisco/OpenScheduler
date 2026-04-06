#ifndef __PROXYAPP_HPP
#define __PROXYAPP_HPP

#include "Console.hpp"
#include "File.hpp"
#include "common.hpp"
#include "dataStructure.hpp"
#include "properties.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <mutex>

class ProxyApp {
public:
  ProxyApp();
  ~ProxyApp();

  int main(int argc, char **argv);
  int readConfig();
  int loadDefaults();
  void printElementNames(xmlNode *a_node);
  int overrideCommandLine();
  int setupDirectory();

  void segnali(int segnale, siginfo_t *info, void *bo);
  void segnaleChld(siginfo_t *info, void *bo);
  void segnaleHup(siginfo_t *info, void *bo);
  void segnaleUsr1(siginfo_t *info, void *bo);
  void segnaleBus(siginfo_t *info, void *bo);

  String nomeFileDaCodice(String codice);

private:
  int rumble();
  int runCommand();
  int runCommandsFromFile();
  int mainLoop();
  int mainLoopRunner();
  int testConfig(File &temp, String &error);

  int runConsole(const File *fileScript = nullptr);
  int cmd_help(const StringVector &args);
  int cmd_defval(const StringVector &args);
  int cmd_set(const StringVector &args);
  int cmd_create(const StringVector &args);
  int cmd_list(const StringVector &args);
  int cmd_stamp(const StringVector &args);
  int cmd_stampfile(const StringVector &args);
  int cmd_dump(const StringVector &args);
  int cmd_dumpfile(const StringVector &args);

  int dumpFile(const File &toStamp, const StringVector &args);
  int stampFile(const File &toStamp, const StringVector &args);

  void __registerConsoleCommands();
  void __registerCommandItem(ConsoleCommandVector &cmdarray, String commandName,
                             String helpCmd, String helpDescr,
                             CommandFunction function,
                             CommandFunction completeFunction);

public:
  String configFile, workPath, scriptFile;
  int verbose;
  bool buildDir;

  xmlDoc *doc;
  xmlNode *root_element;
  File workDir, slotDir, logsDir;
  std::mutex mtxRunner;
  StringMap requestHeaders;
  SlotFile defslot;
  AnyStringMap defstamper;
  StringVector directCommand;
  ConsoleCommandVector basicCommands;
};

#define BEGIN_COMMAND_LIST()                                                   \
  void ProxyApp::__registerConsoleCommands() {                                 \
    ConsoleCommandVector &cmdarray = basicCommands;

#define COMMAND_ITEM(nome, hpar, hdesc)                                        \
  __registerCommandItem(                                                       \
      cmdarray, #nome, hpar, hdesc,                                            \
      [this](const Arguments &args) { return this->cmd_##nome(args); },        \
      nullptr);

#define COMMAND_ITEM2(nome, hpar, hdesc)                                       \
  __registerCommandItem(                                                       \
      cmdarray, #nome, hpar, hdesc,                                            \
      [this](const Arguments &args) { return this->cmd_##nome(args); },        \
      [this](const Arguments &args) { return this->complete_##nome(args); });

#define END_COMMAND_LIST() }

#endif
