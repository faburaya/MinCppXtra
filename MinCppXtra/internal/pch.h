// pch.h: Dies ist eine vorkompilierte Headerdatei.
// Die unten aufgeführten Dateien werden nur einmal kompiliert, um die Buildleistung für zukünftige Builds zu verbessern.
// Dies wirkt sich auch auf die IntelliSense-Leistung aus, Codevervollständigung und viele Features zum Durchsuchen von Code eingeschlossen.
// Die hier aufgeführten Dateien werden jedoch ALLE neu kompiliert, wenn mindestens eine davon zwischen den Builds aktualisiert wird.
// Fügen Sie hier keine Dateien hinzu, die häufig aktualisiert werden sollen, da sich so der Leistungsvorteil ins Gegenteil verkehrt.

#ifndef PCH_H
#define PCH_H

// Fügen Sie hier Header hinzu, die vorkompiliert werden sollen.
#include "framework.h"
#include <windows.h>

#undef min
#undef max

#define NOT_OK(call) (call) == FALSE
#define OK(call) (call) == TRUE
#define NAMEOF(x) #x

namespace mincpp
{
	/// <summary>
	/// Gets a handle for this process that can be used as process handle for some Win32 API calls.
	/// See https://learn.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-syminitialize#parameters.
	/// </summary>
	/// <returns>A handle for the current process.</returns>
	HANDLE GetThisProcessHandle();
}

#endif //PCH_H
