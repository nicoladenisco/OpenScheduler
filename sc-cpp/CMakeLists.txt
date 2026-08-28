cmake_minimum_required(VERSION 3.30)
project(oskcli VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- 1. Trova le dipendenze (tutte insieme all'inizio) ---
# Boost.System e' header-only: da Boost 1.90 la libreria compilata non esiste piu'
find_package(Boost REQUIRED CONFIG COMPONENTS thread date_time regex program_options chrono)
find_package(Threads REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(JNI REQUIRED) # <--- AGGIUNTO QUESTO

find_library(READLINE_LIBRARY NAMES readline)
find_library(NCURSES_LIBRARY NAMES ncurses tinfo)

# --- 2. Definizione dei Sorgenti ---
set(CLI_SRCS
  src/main.cpp   # Messo qui il main
  src/common.cpp
  src/ProxyApp.cpp
  src/XmlHelper.cpp
  src/File.cpp
  src/dataStructure.cpp
  src/SchedSlotBase.cpp
  src/SchedResource.cpp
  src/SchedStamper.cpp
  src/SchedStamperAlgo.cpp
  src/SchedMerger.cpp
  src/SchedMergerAlgo.cpp
  src/Console.cpp
  src/SimpleTimer.cpp
)

set(LIB_SRCS
  src/common.cpp
  src/XmlHelper.cpp
  src/File.cpp
  src/dataStructure.cpp
  src/SchedSlotBase.cpp
  src/SchedResource.cpp
  src/SchedStamper.cpp
  src/SchedStamperAlgo.cpp
  src/SchedMerger.cpp
  src/SchedMergerAlgo.cpp
  src/SimpleTimer.cpp
  src/JNI_adapter.cpp
)

# --- 3. Creazione Target (Prima si creano, poi si configurano) ---

# Creazione Eseguibile
add_executable(oskcli ${CLI_SRCS})

# Creazione Libreria
add_library(oskcore SHARED ${LIB_SRCS})
set_target_properties(oskcore PROPERTIES POSITION_INDEPENDENT_CODE ON)


# --- 4. Configurazione Eseguibile (oskcli) ---
target_link_libraries(oskcli PRIVATE
    Boost::headers Boost::thread Boost::date_time
    Boost::regex Boost::program_options Boost::chrono
    Threads::Threads
    LibXml2::LibXml2
    ${READLINE_LIBRARY} ${NCURSES_LIBRARY}
)
target_link_options(oskcli PRIVATE "-no-pie")


# --- 5. Configurazione Libreria (oskcore) ---

# Spostato qui dopo add_library
target_include_directories(oskcore PRIVATE
    src
    ${JNI_INCLUDE_DIRS}
)

target_link_libraries(oskcore PRIVATE
    Boost::headers Boost::thread Boost::date_time
    Boost::regex Boost::program_options Boost::chrono
    Threads::Threads
    LibXml2::LibXml2
    ${READLINE_LIBRARY} ${NCURSES_LIBRARY}
    ${JNI_LIBRARIES}
)

# Se vuoi che chi usa la tua libreria sappia dove sono gli header:
target_include_directories(oskcore PUBLIC src)

