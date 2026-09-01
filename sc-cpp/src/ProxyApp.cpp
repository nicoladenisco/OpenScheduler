#include "ProxyApp.hpp"
#include "Console.hpp"
#include "File.hpp"
#include "Properties.hpp"
#include "SchedMerger.hpp"
#include "SchedResource.hpp"
#include "SchedStamper.hpp"
#include "SimpleTimer.hpp"
#include "XmlHelper.hpp"
#include "common.hpp"
#include "dataStructure.hpp"
#include <algorithm>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/config.hpp>
#include <boost/program_options/environment_iterator.hpp>
#include <boost/program_options/eof_iterator.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/version.hpp>
#include <fstream>
#include <unistd.h>

#define LOCK_RUNNER std::lock_guard<std::mutex> lck(mtxRunner);

extern ProxyApp theApp;
namespace po = boost::program_options;

BEGIN_COMMAND_LIST()
COMMAND_ITEM(help, 1, "visualizza help", "visualizza comandi disponibili")
COMMAND_ITEM(defval, 1, "defval", "visualizza defaults")
COMMAND_ITEM(set, 3, "set <nome campo> <valore>",
             "inserisce il valore nel campo indicato")
COMMAND_ITEM(
    create, 2, "create <codice> [nomefile]",
    "crea un file risorsa con il codice e il nomefile indicato (opzionale)")
COMMAND_ITEM(list, 1, "list [scan][verbose]",
             "visualizza slot nell'area corrente")
COMMAND_ITEM2(stamp, 3, "stamp <codice> <algo> [parameters algo]",
              "inizializza la risorsa con l'algoritmo indicato")
COMMAND_ITEM2(stampfile, 3, "stampfile <nomefile> <algo> [parameters algo]",
              "come stamp ma con indicazione esplicita del nome file")
COMMAND_ITEM2(dump, 2, "dump <codice> [dayStart] [dayStop] [fileoutput]",
              "dump della risorsa con il codice indicato")
COMMAND_ITEM2(dumpfile, 2,
              "dumpfile <nomefile> [dayStart] [dayStop] [fileoutput]",
              "come dump ma con indicazione esplicita del nome file")
COMMAND_ITEM2(dumpxml, 2, "dumpxml <codice> [dayStart] [dayStop] [fileoutput]",
              "dump della risorsa con il codice indicato in formato XML")
COMMAND_ITEM2(dumpfilexml, 2,
              "dumpfilexml <nomefile> [dayStart] [dayStop] [fileoutput]",
              "come dumpxml ma con indicazione esplicita del nome file")
COMMAND_ITEM2(merge, 3, "merge <codice> <algo> [parameters algo]",
              "merge della risorsa")
COMMAND_ITEM(dumpmerge, 1, "dumpmerge [dayStart] [dayStop] [fileoutput]",
             "dump della fusione corrente")
COMMAND_ITEM(dumpmergexml, 1, "dumpmergexml [dayStart] [dayStop] [fileoutput]",
             "dump della fusione corrente in formato XML")
COMMAND_ITEM(infomerge, 1, "infomerge", "informazioni della fusione corrente")
COMMAND_ITEM(clearmerge, 1, "clearmerge", "pulisce la fusione corrente")
END_COMMAND_LIST()

ProxyApp::ProxyApp()
    : configFile("config.xml"), verbose(0), workPath("/tmp/euridice"),
      merger(getpid())
{
  doc = NULL;
  root_element = NULL;
  __registerConsoleCommands();

  if (strStartWith(workPath, "/tmp"))
    buildDir = true;

  initSlotFile(2026, 4, 8, 18, "DUMMY", defslot);

  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  LIBXML_TEST_VERSION
}

ProxyApp::~ProxyApp()
{
  if (doc != NULL)
  {
    // free the document
    xmlFreeDoc(doc);
  }
}

int ProxyApp::main(int argc, char **argv)
{
  // Declare the supported options.
  po::options_description desc("Allowed options");
  desc.add_options()("help", "produce help message")("verbose",
                                                     "produce more output")(
      "workdir", po::value<std::string>(&workPath)->default_value(workPath),
      "set working directory")("builddir", "create working directories")(
      "config", po::value<std::string>(&configFile)->default_value(configFile),
      "set config file")("cmd", "execute command; all parameters on the right "
                                "are command and its arguments")(
      "cmdfile", po::value<std::string>(&scriptFile),
      "execute commands from file like console");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
  {
    cout << desc << "\n";
    return 1;
  }

  verbose = vm.count("verbose");
  if (!buildDir)
    buildDir = vm.count("builddir");

  // recupera tutti i parametri dopo --cmd
  if (vm.count("cmd"))
  {
    for (int i = 0; i < argc; i++)
    {
      if (strcmp(argv[i], "--cmd") == 0)
      {
        for (int j = i + 1; j < argc; j++)
        {
          directCommand.push_back(argv[j]);
        }
        if (directCommand.empty())
        {
          cout << "Invalid parameters for --cmd; must specify command and its "
                  "arguments\n";
          return -1;
        }
        break;
      }
    }
  }

  return rumble();
}

int ProxyApp::readConfig()
{
  /* parse the file and get the DOM */
  doc = xmlReadFile(configFile.c_str(), NULL, 0);

  if (doc == NULL)
  {
    printf("error: could not parse file %s\n", configFile.c_str());
    return -1;
  }

  /* Get the root element node */
  root_element = xmlDocGetRootElement(doc);

  if (verbose)
    printElementNames(root_element);

  /*
   * Free the global variables that may
   * have been allocated by the parser.
   */
  xmlCleanupParser();

  return 0;
}

/**
 * print_element_names:
 * @a_node: the initial xml node to consider.
 *
 * Prints the names of the all the xml elements
 * that are siblings or children of a given xml node.
 */
void ProxyApp::printElementNames(xmlNode *a_node)
{
  xmlNode *cur_node = NULL;

  for (cur_node = a_node; cur_node; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      printf("node type: Element, name: %s\n", cur_node->name);
    }

    printElementNames(cur_node->children);
  }
}

int ProxyApp::rumble()
{
  try
  {
    if (readConfig())
      return -1;

    if (loadDefaults())
      return -1;

    if (overrideCommandLine())
      return -1;

    if (setupDirectory())
      return -1;

    if (scanArea())
      return -1;

    if (!directCommand.empty())
    {
      return runCommand();
    }

    if (!scriptFile.empty())
    {
      return runCommandsFromFile();
    }

    // avvia colloquio con utente
    if (mainLoop())
      return -1;

    return 0;
  }
  catch (std::exception &e)
  {
    fprintf(stderr, "Fatal error: %s\n", e.what());
    return -1;
  }
  catch (...)
  {
    fprintf(stderr, "Fatal error: unknow cause\n");
    return -1;
  }
}

int ProxyApp::loadDefaults()
{
  XmlHelper xh(root_element);
  const xmlNode *defaults = xh.findElementXml("defaults");
  if (defaults == NULL)
    return 0;

  XmlHelper dh(defaults);
  const xmlNode *stamper = dh.findElementXml("stamper");
  if (stamper != NULL)
  {
    XmlHelper sh(stamper);
    NodeVector params = sh.getChildren("param");
    for (auto p : params)
    {
      XmlHelper ph(p);
      String name = ph.getAttribute("name");
      String value = ph.getAttribute("value");
      defstamper[name] = value;
    }
  }

  return 0;
}

int ProxyApp::overrideCommandLine()
{
  XmlHelper xh(root_element);
  const xmlNode *override = xh.findElementXml("override");
  if (override == NULL)
    return 0;

  XmlHelper xo(override);
  // xo.findElementXmlContent("host", lifeconHost);
  return 0;
}

int ProxyApp::setupDirectory()
{
  String main("/tmp/euridice");
  String slot = main + "/slots";
  String logs = main + "/logs";

  XmlHelper xh(root_element);
  const xmlNode *dirs = xh.findElementXml("dirs");
  if (dirs != NULL)
  {
    XmlHelper xd(dirs);
    xd.findElementXmlContent("main", main);
    xd.findElementXmlContent("slot", slot);
    xd.findElementXmlContent("logs", logs);
  }

  {
    File test(main);
    if (!test.isDirectory() && !buildDir)
      throw FileException(format(
          "Directory %s inesistente o non è una directory.", main.c_str()));

    workDir = test;
    workDir.mkdirs();
  }

  {
    File test(slot);
    if (!test.isDirectory() && !buildDir)
      throw FileException(format(
          "Directory %s inesistente o non è una directory.", slot.c_str()));

    slotDir = test;
    slotDir.mkdirs();
  }

  {
    File test(logs);
    if (!test.isDirectory() && !buildDir)
      throw FileException(format(
          "Directory %s inesistente o non è una directory.", logs.c_str()));

    logsDir = test;
    logsDir.mkdirs();
  }

  return 0;
}

void ProxyApp::segnali(int segnale, siginfo_t *info, void *bo)
{
  switch (segnale)
  {
  case SIGCHLD:
    segnaleChld(info, bo);
    break;

  case SIGHUP:
    segnaleHup(info, bo);
    break;

  case SIGUSR1:
    segnaleUsr1(info, bo);
    break;

  case SIGBUS:
    segnaleBus(info, bo);
    break;
  }
}

void ProxyApp::segnaleChld(siginfo_t *info, void *bo)
{
  try
  {
    // se il PID non è associato ad una istanza in esecuzione cattura qui
    int exitCode = 0;
    int pid = waitpid(0, &exitCode, WNOHANG);
    if (pid)
      cout << "unqualifed child pid=" << pid
           << " defunct (exitCode=" << exitCode << ")\n";
  }
  catch (std::exception &e)
  {
    fprintf(stderr, "SIGCHLD error: %s\n", e.what());
  }
  catch (...)
  {
    fprintf(stderr, "SIGCHLD error: unknow cause\n");
  }
}

void ProxyApp::segnaleHup(siginfo_t *info, void *bo)
{
  cout << "Segnale HUP aggiornamento configurazione ricevuto.\n";

  try
  {
    LOCK_RUNNER

    if (verbose)
      cout << "Segnale HUP aggiornamento configurazione eseguito.\n";
  }
  catch (std::exception &e)
  {
    fprintf(stderr, "SIGHUP error: %s\n", e.what());
  }
  catch (...)
  {
    fprintf(stderr, "SIGHUP error: unknow cause\n");
  }
}

void ProxyApp::segnaleUsr1(siginfo_t *info, void *bo)
{
  cout << "Segnale USR1 dump configurazione.\n";

  try
  {
    LOCK_RUNNER

    if (verbose)
      cout << "Segnale USR1 dump configurazione eseguito.\n";
  }
  catch (std::exception &e)
  {
    fprintf(stderr, "SIGUSR1 error: %s\n", e.what());
  }
  catch (...)
  {
    fprintf(stderr, "SIGUSR1 error: unknow cause\n");
  }
}

void ProxyApp::segnaleBus(siginfo_t *info, void *bo)
{
  cout << "Segnale BUS troncamento inatteso di file mappato in memoria.\n";
}

int ProxyApp::mainLoop()
{
  try
  {
    LOCK_RUNNER
    return mainLoopRunner();
  }
  catch (std::exception &e)
  {
    fprintf(stderr, "Fatal error: %s\n", e.what());
  }
  catch (...)
  {
    fprintf(stderr, "Fatal error: unknow cause\n");
  }

  return -1;
}

int ProxyApp::mainLoopRunner()
{
  runConsole();
  cout << "bye\n";
  return 0;
}

void ProxyApp::__registerCommandItem(ConsoleCommandVector &cmdarray,
                                     String commandName, int minParams,
                                     String helpCmd, String helpDescr,
                                     CommandFunction function,
                                     CommandCompleter completeFunction)
{
  ConsoleCommand cmd;
  cmd.commandName = commandName;
  cmd.minParams = minParams;
  cmd.helpCmd = helpCmd;
  cmd.helpDescr = helpDescr;
  cmd.function = function;
  cmd.completeFunction = completeFunction;
  cmdarray.push_back(cmd);
}

int ProxyApp::runConsole(const File *fileScript /* = nullptr */)
{
  Console c(">");
  c.registerCommands(basicCommands);

  if (fileScript != nullptr && fileScript->isFile())
  {
    c.executeFile(fileScript->getAbsolutePath());
    return 0;
  }

  using ret = Console::ReturnCode;

  int retCode;
  do
  {
    retCode = c.readLine();
  } while (retCode != ret::Quit);

  return 0;
}

int ProxyApp::cmd_help(const StringVector &args)
{
  cout << "Help comandi:\n";
  for (auto cmd : basicCommands)
  {
    cout << "    " << cmd.helpCmd << "\n"
         << "\t- " << cmd.helpDescr << "\n";
  }
  cout << "    run <file comandi>\n"
          "\t- esegue una lista di comandi contenuti in un file di testo\n";
  cout << "    exit/quit\n"
          "\t- esce dal programma\n";
  return 0;
}

int ProxyApp::cmd_defval(const StringVector &args)
{
  cout << toString(defslot) << "\n";
  return 0;
}

int ProxyApp::cmd_set(const StringVector &args) { return 0; }

int ProxyApp::cmd_create(const StringVector &args)
{
  String codice = args[1];
  String nomeFile = nomeFileDaCodice(codice);
  if (args.size() >= 3)
    nomeFile = args[2];

  SlotFile generato;
  File genfile(slotDir, nomeFile);
  if (genfile.isFile())
  {
    cout << "Il file " << genfile.getAbsolutePath()
         << " già esiste! Comando create ignorato.\n";
    return 0;
  }

  initSlotFile(defslot.anno, defslot.slotOra, defslot.oraIniziale,
               defslot.oraFinale, codice, generato, genfile);

  cout << "Generato nuovo file slot:\n"
       << genfile.str() << "\n"
       << toString(generato) << "\n\n";
  return 0;
}

String ProxyApp::nomeFileDaCodice(String codice)
{
  return "Slot_" + codice + ".bin";
}

int ProxyApp::cmd_list(const StringVector &args)
{
  FileVector files;
  slotDir.listFiles(files);

  if (args.size() > 1 && (args[1] == "scan" || args[1] == "verbose"))
  {
    bool showdet = args[1] == "verbose";

    for (auto f : files)
    {
      SlotFile tmp;
      int fd;
      if ((fd = open(f.c_str(), O_RDONLY)) != -1)
      {
        if (read(fd, &tmp, sizeof(tmp)) != sizeof(tmp))
        {
          cout << "Il file " << f.getAbsolutePath() << " è corrotto.\n";
          close(fd);
          continue;
        }
        close(fd);

        if (strncmp(MAGIC, tmp.magic, 2) == 0)
        {
          cout << f.getAbsolutePath() << "\n";

          if (strncmp(FIRMA, tmp.firma, 16) == 0)
          {
            if (showdet)
              cout << toString(tmp) << "\n";
          }
          else
          {
            String ss(tmp.firma);
            cout << "Formato incompatibile: atteso '" << FIRMA << "' letto '"
                 << trim(ss.substr(0, 16)) << "' versione non compatibile.\n";
          }
        }
      }
    }

    return 0;
  }

  for (auto f : files)
    cout << f.getAbsolutePath() << "\n";

  return 0;
}

int ProxyApp::runCommand()
{
  for (auto cmd : basicCommands)
  {
    if (directCommand[0] == cmd.commandName)
    {
      // this->directCommand.erase(this->directCommand.begin());
      return cmd.function(this->directCommand);
    }
  }

  cout << "unknow command " << directCommand[0] << "\n";
  return -1;
}

int ProxyApp::runCommandsFromFile()
{
  File fileScript(scriptFile);
  if (!fileScript.isFile())
  {
    cout << "Script file " << scriptFile << " not exists.\n";
    return -1;
  }
  return runConsole(&fileScript);
}

int ProxyApp::cmd_stamp(const StringVector &args)
{
  String codice = args[1];
  String nomeFile = nomeFileDaCodice(codice);
  File genfile(slotDir, nomeFile);
  if (!genfile.isFile())
  {
    cout << "La risorsa con codice " << codice
         << " non ha un corrispondente file in " << slotDir.getAbsolutePath()
         << "\n";
    return -1;
  }

  return stampFile(genfile, args);
}

int ProxyApp::cmd_stampfile(const StringVector &args)
{
  String nomeFile = args[1];
  File genfile(slotDir, nomeFile);
  if (!genfile.isFile())
  {
    cout << "Il file indicato non esiste in " << slotDir.getAbsolutePath()
         << "\n";
    return -1;
  }

  return stampFile(genfile, args);
}

int ProxyApp::stampFile(const File &toStamp, const StringVector &args)
{
  SchedStamper stamper;
  String algo = args[2];

  // verifica per algoritmo esistente
  StringVector names;
  stamper.getAlgoNames(names);
  if (find(names.begin(), names.end(), algo) == names.end())
  {
    cout << "Algoritmo " << algo << " inesistente: deve essere uno di "
         << join(names, ",", "'") << "\n";
    return 0;
  }

  SimpleTimer st;

  // carica risorsa e applica stamper
  SchedResource res(toStamp);
  Properties properties = defstamper;
  properties.vector2Properties(args);

  // lock della risorsa
  SchedResourceLock reslock(res, "stamp", true, true, 3000);
  if (!reslock.isLocked())
  {
    cout << "Non riesco a bloccare la risorsa; operazione abortita.\n";
    return 0;
  }

  stamper.stampResource(res, algo, properties);
  st.showElapsed("Stamper ");

  return 0;
}

int ProxyApp::cmd_dump(const StringVector &args)
{
  String codice = args[1];
  String nomeFile = nomeFileDaCodice(codice);
  File genfile(slotDir, nomeFile);
  if (!genfile.isFile())
  {
    cout << "La risorsa con codice " << codice
         << " non ha un corrispondente file in " << slotDir.getAbsolutePath()
         << "\n";
    return -1;
  }

  return dumpFile(genfile, args);
}

int ProxyApp::cmd_dumpfile(const StringVector &args)
{
  String nomeFile = args[1];
  File genfile(slotDir, nomeFile);
  if (!genfile.isFile())
  {
    cout << "Il file indicato non esiste in " << slotDir.getAbsolutePath()
         << "\n";
    return -1;
  }

  return dumpFile(genfile, args);
}

int ProxyApp::dumpFile(const File &toDump, const StringVector &args)
{
  cout << "File: " << toDump.getAbsolutePath() << "\n";

  IntPair days = parseDays(args, 2);
  String fileoutput;
  if (args.size() >= 5)
    fileoutput = args[4];

  // carica risorsa e applica stamper
  SchedResource res(toDump);
  cout << toString(*res.getSlotFile()) << "\n";

  // lock della risorsa
  SchedResourceLock reslock(res, "dump", true, true, 3000);
  if (!reslock.isLocked())
  {
    cout << "Non riesco a bloccare la risorsa; operazione abortita.\n";
    return 0;
  }

  if (res.isInitialized())
  {
    String output = dump(*res.getSlotFile(), days.first, days.second);
    if (fileoutput.empty())
      cout << output << "\n";
    else
    {
      cout << "Output inviato a " << fileoutput << "\n";

      std::ofstream outputFile(fileoutput, std::ios::binary | std::ios::trunc);
      if (!outputFile)
      {
        cout << "Non riesco ad aprire il file di output " << fileoutput << "\n";
        return 0;
      }

      outputFile.write(output.data(),
                       static_cast<std::streamsize>(output.size()));
      outputFile.put('\n');
      if (!outputFile)
      {
        cout << "Errore durante la scrittura del file di output " << fileoutput
             << "\n";
        return 0;
      }
    }
  }
  else
    cout << "La risorsa non è stata inizializzata; usare uno stamper per "
            "poterla usare.\n";

  return 0;
}

int ProxyApp::complete_stamp(const StringVector &args, StringVector &complete,
                             int np)
{
  if (np == 1)
    resourcesFromArea(complete);

  if (np == 2)
  {
    SchedStamper stamper;
    stamper.getAlgoNames(complete);
  }

  return 0;
}

int ProxyApp::complete_stampfile(const StringVector &args,
                                 StringVector &complete, int np)
{
  return 0;
}

int ProxyApp::complete_dump(const StringVector &args, StringVector &complete,
                            int np)
{
  if (np == 1)
    resourcesFromArea(complete);
  return 1;
}

int ProxyApp::complete_dumpfile(const StringVector &args,
                                StringVector &complete, int np)
{
  if (np == 1)
    filesFromArea(complete);
  return 1;
}

int ProxyApp::complete_dumpxml(const StringVector &args, StringVector &complete,
                               int np)
{
  if (np == 1)
    resourcesFromArea(complete);
  return 1;
}

int ProxyApp::complete_dumpfilexml(const StringVector &args,
                                   StringVector &complete, int np)
{
  if (np == 1)
    filesFromArea(complete);
  return 1;
}

void ProxyApp::resourcesFromArea(StringVector &rv)
{
  for (auto it : cacheRisorse)
  {
    rv.push_back(it.first);
  }
}

void ProxyApp::filesFromArea(StringVector &rv)
{
  for (auto it : cacheRisorse)
  {
    for (auto f : it.second)
      rv.push_back(f.getAbsolutePath());
  }
}

int ProxyApp::cmd_merge(const StringVector &args)
{
  String codice = args[1];
  String algo = args[2];

  if (merger.checkRisorsa(codice))
  {
    cout << "La risorsa con codice " << codice << " è stata già inclusa.\n";
    return 0;
  }

  String nomeFile = nomeFileDaCodice(codice);
  File genfile(slotDir, nomeFile);
  if (!genfile.isFile())
  {
    cout << "La risorsa con codice " << codice
         << " non ha un corrispondente file in " << slotDir.getAbsolutePath()
         << "\n";
    return 0;
  }

  // verifica per algoritmo esistente
  StringVector names;
  merger.getAlgoNames(names);
  if (find(names.begin(), names.end(), algo) == names.end())
  {
    cout << "Algoritmo " << algo << " inesistente: deve essere uno di "
         << join(names, ",", "'") << "\n";
    return 0;
  }

  // carica risorsa
  SchedResourcePtr res = buildResource(genfile);
  if (!res->isInitialized())
  {
    cout << "La risorsa non è stata inizializzata; usare uno stamper per "
            "poterla usare.\n";
    return 0;
  }
  cout << toString(*res->getSlotFile()) << "\n";

  SimpleTimer st;
  Properties properties;
  properties.vector2Properties(args);
  // cout << properties.toString() << "\n";
  SchedResourceMultiLock multilock("merge", true, true, 5000);
  merger.addResource(multilock, res, algo, properties);
  st.showElapsed("Merge");
  return 0;
}

int ProxyApp::complete_merge(const StringVector &args, StringVector &complete,
                             int np)
{
  if (np == 1)
    resourcesFromArea(complete);

  if (np == 2)
    merger.getAlgoNames(complete);

  return 0;
}

int ProxyApp::scanArea()
{
  SimpleTimer st;
  cout << "Scan directory " << slotDir.getAbsolutePath() << " for data.\n";

  FileVector files;
  slotDir.listFiles(files);

  for (auto f : files)
  {
    SlotFile tmp;
    int fd;
    if ((fd = open(f.c_str(), O_RDONLY)) != -1)
    {
      if (read(fd, &tmp, sizeof(tmp)) != sizeof(tmp))
      {
        close(fd);
        continue;
      }
      close(fd);

      if (strncmp(MAGIC, tmp.magic, 2) == 0)
      {
        if (strncmp(FIRMA, tmp.firma, 16) == 0)
        {
          cacheRisorse.aggiungi(tmp.codiceRisorsa, f);
        }
      }
    }
  }

  st.showElapsed("Scan area");
  return 0;
}

int ProxyApp::cmd_dumpmerge(const StringVector &args)
{
  const SlotFile *ptSlot = merger.getMerged();

  if (ptSlot == nullptr)
  {
    cout << "Il merger è vuoto\n";
    return 0;
  }

  IntPair days = parseDays(args, 1);
  String fileoutput;
  if (args.size() >= 4)
    fileoutput = args[3];

  String output = dump(*ptSlot, days.first, days.second);

  if (fileoutput.empty())
    cout << output << "\n";
  else
  {
    cout << "Output inviato a " << fileoutput << "\n";

    std::ofstream outputFile(fileoutput, std::ios::binary | std::ios::trunc);
    if (!outputFile)
    {
      cout << "Non riesco ad aprire il file di output " << fileoutput << "\n";
      return 0;
    }

    outputFile.write(output.data(),
                     static_cast<std::streamsize>(output.size()));
    outputFile.put('\n');
    if (!outputFile)
    {
      cout << "Errore durante la scrittura del file di output " << fileoutput
           << "\n";
      return 0;
    }
  }

  return 0;
}

int ProxyApp::cmd_dumpmergexml(const StringVector &args)
{
  const SlotFile *ptSlot = merger.getMerged();

  if (ptSlot == nullptr)
  {
    cout << "Il merger è vuoto\n";
    return 0;
  }

  IntPair days = parseDays(args, 1);
  String fileoutput;
  if (args.size() >= 4)
    fileoutput = args[3];

  Properties prop;
  prop["daystart"] = days.first;
  prop["daystop"] = days.second;
  String output = merger.dumpXml(prop);

  if (fileoutput.empty())
    cout << output << "\n";
  else
  {
    cout << "Output inviato a " << fileoutput << "\n";

    std::ofstream outputFile(fileoutput, std::ios::binary | std::ios::trunc);
    if (!outputFile)
    {
      cout << "Non riesco ad aprire il file di output " << fileoutput << "\n";
      return 0;
    }

    String hxml = "<?xml version=\"1.0\"?>\n";
    outputFile.write(hxml.data(), static_cast<std::streamsize>(hxml.size()));
    outputFile.write(output.data(),
                     static_cast<std::streamsize>(output.size()));
    outputFile.put('\n');
    if (!outputFile)
    {
      cout << "Errore durante la scrittura del file di output " << fileoutput
           << "\n";
      return 0;
    }
  }

  return 0;
}

int ProxyApp::cmd_infomerge(const StringVector &args)
{
  cout << merger.toString() << "\n";
  return 0;
}

int ProxyApp::cmd_clearmerge(const StringVector &args)
{
  merger.clear();
  cout << "Accorpamento risorse svuotato.\n";
  return 0;
}

int ProxyApp::cmd_dumpxml(const StringVector &args)
{
  String codice = args[1];
  String nomeFile = nomeFileDaCodice(codice);
  File genfile(slotDir, nomeFile);
  if (!genfile.isFile())
  {
    cout << "La risorsa con codice " << codice
         << " non ha un corrispondente file in " << slotDir.getAbsolutePath()
         << "\n";
    return -1;
  }

  return dumpFileXML(genfile, args);
}

int ProxyApp::cmd_dumpfilexml(const StringVector &args)
{
  String nomeFile = args[1];
  File genfile(slotDir, nomeFile);
  if (!genfile.isFile())
  {
    cout << "Il file indicato non esiste in " << slotDir.getAbsolutePath()
         << "\n";
    return -1;
  }

  return dumpFileXML(genfile, args);
}

int ProxyApp::dumpFileXML(const File &toDump, const StringVector &args)
{
  cout << "File: " << toDump.getAbsolutePath() << "\n";

  IntPair days = parseDays(args, 2);
  String fileoutput;
  if (args.size() >= 5)
    fileoutput = args[4];

  // carica risorsa e applica stamper
  SchedResource res(toDump);
  cout << toString(*res.getSlotFile()) << "\n";

  // lock della risorsa
  SchedResourceLock reslock(res, "dump", true, true, 3000);
  if (!reslock.isLocked())
  {
    cout << "Non riesco a bloccare la risorsa; operazione abortita.\n";
    return 0;
  }

  if (res.isInitialized())
  {
    Properties prop;
    prop["daystart"] = days.first;
    prop["daystop"] = days.second;
    String output = res.dumpXml(prop);

    if (fileoutput.empty())
      cout << output << "\n";
    else
    {
      cout << "Output inviato a " << fileoutput << "\n";

      std::ofstream outputFile(fileoutput, std::ios::binary | std::ios::trunc);
      if (!outputFile)
      {
        cout << "Non riesco ad aprire il file di output " << fileoutput << "\n";
        return 0;
      }

      String hxml = "<?xml version=\"1.0\"?>\n";
      outputFile.write(hxml.data(), static_cast<std::streamsize>(hxml.size()));
      outputFile.write(output.data(),
                       static_cast<std::streamsize>(output.size()));
      outputFile.put('\n');
      if (!outputFile)
      {
        cout << "Errore durante la scrittura del file di output " << fileoutput
             << "\n";
        return 0;
      }
    }
  }
  else
    cout << "La risorsa non è stata inizializzata; usare uno stamper per "
            "poterla usare.\n";

  return 0;
}
