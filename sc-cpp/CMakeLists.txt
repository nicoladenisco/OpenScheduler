cmake_minimum_required(VERSION 3.30)
project(euridice VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## Sorgenti
set(TEST_SRCS
  src/main.cpp
  src/common.cpp
  src/ProxyApp.cpp
  src/XmlHelper.cpp
  src/File.cpp
  src/dataStructure.cpp
  src/SchedResource.cpp
  src/SchedStamper.cpp
  src/SchedStamperAlgo.cpp
  src/SchedMerger.cpp
  src/SchedMergerAlgo.cpp
  src/Console.cpp
  src/SimpleTimer.cpp
)

add_executable(euridice ${TEST_SRCS})

## 1. Boost
# Rimosso 'any' dai componenti perché è solo header
set(BOOST_LIBS thread date_time system regex program_options chrono)

# Cerca Boost (CONFIG mode è la scelta migliore con CMake 3.30+)
find_package(Boost REQUIRED CONFIG COMPONENTS ${BOOST_LIBS})

# Link moderno: i target Boost::[nome] gestiscono automaticamente anche gli include
target_link_libraries(euridice PRIVATE
    Boost::headers         # Questo abilita tutte le librerie header-only come 'any'
    Boost::thread
    Boost::date_time
    Boost::system
    Boost::regex
    Boost::program_options
    Boost::chrono
)

## 2. Threads
find_package(Threads REQUIRED)
target_link_libraries(euridice PRIVATE Threads::Threads)

## 3. LibXml2
find_package(LibXml2 REQUIRED)
target_link_libraries(euridice PRIVATE LibXml2::LibXml2)

## 4. Readline
# Cerca la libreria readline e ncurses (necessaria per readline)
find_library(READLINE_LIBRARY NAMES readline)
find_library(NCURSES_LIBRARY NAMES ncurses tinfo)

if(NOT READLINE_LIBRARY)
    message(FATAL_ERROR "Libreria readline non trovata!")
endif()

target_link_libraries(euridice PRIVATE ${READLINE_LIBRARY} ${NCURSES_LIBRARY})

# Soluzione per l'errore "relocation against ... in read-only section"
# Questo disabilita PIE che è spesso la causa di quell'errore specifico con readline
target_link_options(euridice PRIVATE "-no-pie")

