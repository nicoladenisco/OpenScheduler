#include "Console.hpp"
#include "common.hpp"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <sstream>
#include <unordered_map>

#include <cstdlib>

extern "C" {
#include <readline/history.h>
#include <readline/readline.h>
}

Console *currentConsole = nullptr;
HISTORY_STATE *emptyHistory = history_get_history_state();

struct Console::Impl {
  using RegisteredCommands = std::unordered_map<String, CommandFunction>;
  using RegisteredCommandsExtended = std::unordered_map<String, ConsoleCommand>;

  String greeting_;
  // These are hardcoded commands. They do not do anything and are catched
  // manually in the executeCommand function.
  RegisteredCommands commands_;
  RegisteredCommandsExtended extcmds_;
  HISTORY_STATE *history_ = nullptr;

  Impl(String const &greeting) : greeting_(greeting), commands_() {}
  ~Impl() { free(history_); }

  Impl(Impl const &) = delete;
  Impl(Impl &&) = delete;
  Impl &operator=(Impl const &) = delete;
  Impl &operator=(Impl &&) = delete;
};

// Here we set default commands, they do nothing since we quit with them
// Quitting behaviour is hardcoded in readLine()
Console::Console(String const &greeting) : pimpl_{new Impl{greeting}} {
  // Init readline basics
  rl_attempted_completion_function = &Console::getCommandCompletions;

  // These are default hardcoded commands.
  // Help command lists available commands.
  pimpl_->commands_["help"] = [this](const Arguments &) {
    auto commands = getRegisteredCommands();
    cout << "Available commands are:\n";
    for (auto &command : commands)
      cout << "\t" << command << "\n";
    return ReturnCode::Ok;
  };

  // Run command executes all commands in an external file.
  pimpl_->commands_["run"] = [this](const Arguments &input) {
    if (input.size() < 2) {
      cout << "Usage: " << input[0] << " script_filename\n";
      return 1;
    }
    return executeFile(input[1]);
  };

  // Quit and Exit simply terminate the console.
  pimpl_->commands_["quit"] = [this](const Arguments &) {
    return ReturnCode::Quit;
  };

  pimpl_->commands_["exit"] = [this](const Arguments &) {
    return ReturnCode::Quit;
  };
}

Console::~Console() = default;

void Console::registerCommand(const String &s, CommandFunction f) {
  pimpl_->commands_[s] = f;
}

void Console::registerCommand(ConsoleCommand cc) {
  pimpl_->commands_[cc.commandName] = cc.function;
  pimpl_->extcmds_[cc.commandName] = cc;
}

void Console::registerCommands(const ConsoleCommandVector &commands) {
  for (auto cmd : commands) {
    registerCommand(cmd);
  }
}

std::vector<String> Console::getRegisteredCommands() const {
  std::vector<String> allCommands;
  for (auto &pair : pimpl_->commands_)
    allCommands.push_back(pair.first);

  return allCommands;
}

void Console::saveState() {
  free(pimpl_->history_);
  pimpl_->history_ = history_get_history_state();
}

void Console::reserveConsole() {
  if (currentConsole == this)
    return;

  // Save state of other Console
  if (currentConsole)
    currentConsole->saveState();

  // Else we swap state
  if (!pimpl_->history_)
    history_set_history_state(emptyHistory);
  else
    history_set_history_state(pimpl_->history_);

  // Tell others we are using the console
  currentConsole = this;
}

void Console::setGreeting(const String &greeting) {
  pimpl_->greeting_ = greeting;
}

String Console::getGreeting() const { return pimpl_->greeting_; }

int Console::executeCommand(const String &command) {
  // Convert input to vector
  std::vector<String> inputs;
  {
    std::istringstream iss(command);
    std::copy(std::istream_iterator<String>(iss),
              std::istream_iterator<String>(), std::back_inserter(inputs));
  }

  if (inputs.size() == 0)
    return ReturnCode::Ok;

  Impl::RegisteredCommands::iterator it;
  if ((it = pimpl_->commands_.find(inputs[0])) != end(pimpl_->commands_)) {
    return static_cast<int>((it->second)(inputs));
  }

  cout << "Command '" << inputs[0] << "' not found.\n";
  return ReturnCode::Error;
}

int Console::executeFile(const String &filename) {
  std::ifstream input(filename);
  if (!input) {
    cout << "Could not find the specified file to execute.\n";
    return ReturnCode::Error;
  }
  String command;
  int counter = 0, result;

  while (std::getline(input, command)) {
    if (command[0] == '#')
      continue; // Ignore comments
    // Report what the Console is executing.
    if (command != "")
      cout << "[" << counter << "] " << command << '\n';
    if ((result = executeCommand(command)))
      return result;
    ++counter;
    cout << '\n';
  }

  // If we arrived successfully at the end, all is ok
  return ReturnCode::Ok;
}

int Console::readLine() {
  reserveConsole();

  char *buffer = readline(pimpl_->greeting_.c_str());
  if (!buffer) {
    cout << '\n'; // EOF doesn't put last endline so we put that so that it
                  // looks uniform.
    return ReturnCode::Quit;
  }

  // TODO: Maybe add commands to history only if succeeded?
  if (buffer[0] != '\0')
    add_history(buffer);

  String line(buffer);
  free(buffer);

  return executeCommand(line);
}

char **Console::getCommandCompletions(const char *text, int start, int end) {
  char **completionList = nullptr;

  if (start == 0) {
    completionList = rl_completion_matches(text, &Console::commandIterator);
  } else {
    completionList = buildCustomCompletation(text, start, end);
  }

  return completionList;
}

char *Console::commandIterator(const char *text, int state) {
  static Impl::RegisteredCommands::iterator it;
  if (!currentConsole)
    return nullptr;
  auto &commands = currentConsole->pimpl_->commands_;

  if (state == 0)
    it = begin(commands);

  while (it != end(commands)) {
    auto &command = it->first;
    ++it;
    if (command.find(text) != String::npos) {
      return strdup(command.c_str());
    }
  }
  return nullptr;
}

char **Console::buildCustomCompletation(const char *text, int start, int end) {
  StringVector parts;
  split(rl_line_buffer, parts);
  if (parts.size() > 0) {
    printf("\nCOMPLETE: buffer='%s' start=%d %s\n", rl_line_buffer, start,
           text);
    auto cmds = currentConsole->pimpl_->extcmds_;
    auto cmd = cmds.find(trim(parts[0]));
    if (cmd != cmds.end()) {
      printf("COMPLETE FOR: %s\n", cmd->first.c_str());
      return cmd->second.completeFunction(parts);
    }
  }

  return nullptr;
}
