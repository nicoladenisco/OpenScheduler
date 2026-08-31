#ifndef __PROXYAPP_HPP
#define __PROXYAPP_HPP

#include "Classificatore.hpp"
#include "Console.hpp"
#include "File.hpp"
#include "Properties.hpp"
#include "SchedMerger.hpp"
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
  int scanArea();

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
  int cmd_dumpxml(const StringVector &args);
  int cmd_dumpfilexml(const StringVector &args);
  int cmd_merge(const StringVector &args);
  int cmd_dumpmerge(const StringVector &args);
  int cmd_infomerge(const StringVector &args);
  int cmd_clearmerge(const StringVector &args);

  int complete_stamp(const StringVector &args, StringVector &complete, int np);
  int complete_stampfile(const StringVector &args, StringVector &complete,
                         int np);
  int complete_dump(const StringVector &args, StringVector &complete, int np);
  int complete_dumpfile(const StringVector &args, StringVector &complete,
                        int np);
  int complete_dumpxml(const StringVector &args, StringVector &complete,
                       int np);
  int complete_dumpfilexml(const StringVector &args, StringVector &complete,
                           int np);
  int complete_merge(const StringVector &args, StringVector &complete, int np);

  int dumpFile(const File &toStamp, const StringVector &args);
  int dumpFileXML(const File &toStamp, const StringVector &args);
  int stampFile(const File &toStamp, const StringVector &args);

  void resourcesFromArea(StringVector &rv);
  void filesFromArea(StringVector &rv);

  void __registerConsoleCommands();
  void __registerCommandItem(ConsoleCommandVector &cmdarray, String commandName,
                             int minParams, String helpCmd, String helpDescr,
                             CommandFunction function,
                             CommandCompleter completeFunction);

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
  Properties defstamper;
  StringVector directCommand;
  ConsoleCommandVector basicCommands;
  SchedMerger merger;
  Classificatore<String, File> cacheRisorse;
};

#define BEGIN_COMMAND_LIST()                                                   \
  void ProxyApp::__registerConsoleCommands() {                                 \
    ConsoleCommandVector &cmdarray = basicCommands;

#define COMMAND_ITEM(nome, minpar, hpar, hdesc)                                \
  __registerCommandItem(                                                       \
      cmdarray, #nome, minpar, hpar, hdesc,                                    \
      [this](const Arguments &args) { return this->cmd_##nome(args); },        \
      nullptr);

#define COMMAND_ITEM2(nome, minpar, hpar, hdesc)                               \
  __registerCommandItem(                                                       \
      cmdarray, #nome, minpar, hpar, hdesc,                                    \
      [this](const Arguments &args) { return this->cmd_##nome(args); },        \
      [this](const Arguments &args, StringVector &complete, int np) {          \
        return this->complete_##nome(args, complete, np);                      \
      });

#define END_COMMAND_LIST() }

#endif
