////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2016 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Jan Steemann
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGOD_REST_SERVER_CONSOLE_THREAD_H
#define ARANGOD_REST_SERVER_CONSOLE_THREAD_H 1

#include "Basics/Thread.h"
#include "V8Server/ApplicationV8.h"

struct TRI_vocbase_t;

namespace arangodb {
class V8LineEditor;
}

namespace arangodb {
namespace rest {
class ApplicationServer;
}

class ApplicationV8;

////////////////////////////////////////////////////////////////////////////////
/// @brief ConsoleThread
////////////////////////////////////////////////////////////////////////////////

class ConsoleThread : public Thread {
  ConsoleThread(const ConsoleThread&) = delete;
  ConsoleThread& operator=(const ConsoleThread&) = delete;

 public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief the line editor object for use in debugging
  //////////////////////////////////////////////////////////////////////////////

  static arangodb::V8LineEditor* serverConsole;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief mutex for console access
  //////////////////////////////////////////////////////////////////////////////

  static arangodb::Mutex serverConsoleMutex;

 public:
  ConsoleThread(arangodb::rest::ApplicationServer*, ApplicationV8*,
                TRI_vocbase_t*);

  ~ConsoleThread();

 public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief runs the thread
  //////////////////////////////////////////////////////////////////////////////

  void run() override;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief whether or not the thread is chatty on shutdown
  //////////////////////////////////////////////////////////////////////////////

  bool isSilent() override { return true; }

 public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief sets the user abort flag
  //////////////////////////////////////////////////////////////////////////////

  void userAbort() { _userAborted.store(true); }

 private:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief inner thread loop
  //////////////////////////////////////////////////////////////////////////////

  void inner();

 private:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief application server
  //////////////////////////////////////////////////////////////////////////////

  rest::ApplicationServer* _applicationServer;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief V8 dealer
  //////////////////////////////////////////////////////////////////////////////

  ApplicationV8* _applicationV8;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief currently used V8 context
  //////////////////////////////////////////////////////////////////////////////

  ApplicationV8::V8Context* _context;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief vocbase
  //////////////////////////////////////////////////////////////////////////////

  TRI_vocbase_t* _vocbase;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief user aborted flag
  //////////////////////////////////////////////////////////////////////////////

  std::atomic<bool> _userAborted;
};
}

#endif
