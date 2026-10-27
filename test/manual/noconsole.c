/* DUMB: start a Windows program the way Explorer does: no console and
 * no standard handles.  For testing sdldumb.exe's log file and fatal
 * error message box under Wine, which otherwise always passes its own
 * standard handles on.
 *
 * Build: x86_64-w64-mingw32-gcc -O2 -o noconsole.exe noconsole.c
 * Run:   wine noconsole.exe "Z:/path/to/sdldumb.exe -w nosuch.wad"
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <windows.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
   STARTUPINFOA si;
   PROCESS_INFORMATION pi;
   DWORD code;

   if (argc != 2) {
      fprintf(stderr, "usage: noconsole \"PROGRAM ARGS...\"\n");
      return 2;
   }
   ZeroMemory(&si, sizeof(si));
   si.cb = sizeof(si);
   si.dwFlags = STARTF_USESTDHANDLES;
   si.hStdInput = si.hStdOutput = si.hStdError = NULL;
   if (!CreateProcessA(NULL, argv[1], NULL, NULL, FALSE, DETACHED_PROCESS,
		       NULL, NULL, &si, &pi)) {
      printf("CreateProcess failed: %lu\n", GetLastError());
      return 2;
   }
   WaitForSingleObject(pi.hProcess, INFINITE);
   GetExitCodeProcess(pi.hProcess, &code);
   printf("exit code %lu\n", code);
   return 0;
}

// Local Variables:
// c-basic-offset: 3
// End:
