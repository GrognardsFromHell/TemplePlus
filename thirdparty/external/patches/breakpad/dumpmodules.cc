// Copyright (c) 2012 The Mozilla Foundation
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of The Mozilla Foundation nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <algorithm>

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "google_breakpad/processor/basic_source_line_resolver.h"
#include "google_breakpad/processor/minidump.h"
#include "google_breakpad/processor/minidump_processor.h"
#include "google_breakpad/processor/stack_frame_cpu.h"
#include "google_breakpad/processor/system_info.h"
#include "processor/pathname_stripper.h"
#include "processor/simple_symbol_supplier.h"

using namespace google_breakpad;

void error(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);

  exit(1);
}

int main(int argc, char** argv)
{
  if (argc < 2) {
    error("Usage: %s [-v|-d] <minidump>\n\
\t-v\tAlso print debug IDs\n\
\t-d\tPrint relative paths to symbol files that would be loaded", argv[0]);
  }

  const char* dumpname = argv[1];
  bool verbose = false;
  bool debuginfo = false;
  if (strcmp(argv[1], "-v") == 0) {
    dumpname = argv[2];
    verbose = true;
  } else if (strcmp(argv[1], "-d") == 0) {
    dumpname = argv[2];
    debuginfo = true;
  }

  Minidump dump(dumpname);
  if (!dump.Read()) {
    error("Couldn't read minidump %s", argv[1]);
  }

  MinidumpModuleList *modules = dump.GetModuleList();

  if (!modules) {
    error("Minidump %s is missing module list", argv[1]);
  }

  unsigned int module_count = modules->module_count();
  for (unsigned int module_sequence = 0;
       module_sequence < module_count;
       ++module_sequence) {
    const CodeModule *module = modules->GetModuleAtSequence(module_sequence);
    if (debuginfo) {
      string debug_file_name = PathnameStripper::File(module->debug_file());
      string path = debug_file_name;
      path.append("/");
      path.append(module->debug_identifier());
      path.append("/");

      string debug_file_extension;
      if (debug_file_name.size() > 4)
        debug_file_extension = debug_file_name.substr(debug_file_name.size() - 4);
      std::transform(debug_file_extension.begin(), debug_file_extension.end(),
                     debug_file_extension.begin(), tolower);
      if (debug_file_extension == ".pdb") {
        path.append(debug_file_name.substr(0, debug_file_name.size() - 4));
      } else {
        path.append(debug_file_name);
      }
      path.append(".sym");
      printf("%s\n", path.c_str());
    } else {
      printf("%s", module->code_file().c_str());
      if (verbose) {
        printf("\t%s", module->debug_identifier().c_str());
      }
      printf("\n");
    }
  }
  return 0;
}
