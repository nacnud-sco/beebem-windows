/****************************************************************
BeebEm - BBC Micro and Master 128 Emulator
Copyright (C) 2004  Mike Wyatt
Copyright (C) 2004  Rob O'Donnell
Copyright (C) 2009  Steve Pick

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public
License along with this program; if not, write to the Free
Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA  02110-1301, USA.
****************************************************************/

//
// BeebEm debugger
//

#ifndef DEBUG_HEADER
#define DEBUG_HEADER

#include <windows.h>
#include <string>

#include "via.h"

extern bool DebugEnabled;

enum class DebugType {
	None,
	Video,
	UserVIA,
	SysVIA,
	Tube,
	Serial,
	Econet,
	Teletext,
	RemoteServer,
	Manual,
	Breakpoint,
	BRK
};

//*******************************************************************
// Data structs

struct Label
{
	std::string name;
	int addr;

	Label() : addr(0)
	{
	}

	Label(const std::string& n, int a) : name(n), addr(a)
	{
	}
};

struct Breakpoint
{
	int start;
	int end;
	char name[50 + 1];
};

struct Watch
{
	int start;
	char type;
	int value;
	bool host;
	char name[50 + 1];
};

struct InstInfo
{
	const char* opcode;
	int bytes;
	int flag;
};

struct AddrInfo
{
	int start;
	int end;
	char desc[100];
};

struct MemoryMap
{
	AddrInfo* entries;
	int count;
};

struct DebugCmd
{
	char *name;
	bool (*handler)(char* arguments);
	const char *argdesc;
	const char *help;
};

extern HWND hwndDebug;
int DebugDisassembleInstruction(int addr, bool host, char *opstr);
int DebugDisassembleInstructionWithCPUStatus(int addr,
                                             bool host,
                                             unsigned char Accumulator,
                                             unsigned char XReg,
                                             unsigned char YReg,
                                             unsigned char StackReg,
                                             unsigned char PSR,
                                             char *opstr);
void DebugOpenDialog(HINSTANCE hinst, HWND hwndMain);
void DebugCloseDialog(void);
bool DebugDisassembler(int addr, int prevAddr, int Accumulator, int XReg, int YReg, int PSR, int StackReg, bool host);
void DebugDisplayTrace(DebugType type, bool host, const char *info);
void DebugDisplayInfo(const char *info);
void DebugDisplayInfoF(const char *format, ...);
void DebugVideoState(void);
void DebugUserViaState(void);
void DebugSysViaState(void);
void DebugViaState(const char *s, VIAState *v);
void DebugParseCommand(char *command);
void DebugRunScript(const char *filename);
bool DebugLoadSwiftLabels(const char *filename);
unsigned char DebugReadMem(int addr, bool host);
void DebugWriteMem(int addr, bool host, unsigned char data);
int DebugDisassembleInstruction(int addr, bool host, char *opstr);
int DebugDisassembleCommand(int addr, int count, bool host);
void DebugMemoryDump(int addr, int count, bool host);
void DebugExecuteCommand();
void DebugToggleRun();
void DebugBreakExecution(DebugType type);
void DebugUpdateWatches(bool all);
void DebugDisplayInfoF(const char *format, ...);
bool DebugLookupAddress(int addr, AddrInfo* addrInfo);
void DebugHistoryMove(int delta);
void DebugHistoryAdd(char* command);
void DebugInitMemoryMaps();
bool DebugLoadMemoryMap(char* filename, int bank);
void DebugSetCommandString(char* string);
void DebugChompString(char* string);

bool DebugCmdBreakContinue(char* args);
bool DebugCmdToggleBreak(char* args);
bool DebugCmdLabels(char* args);
bool DebugCmdHelp(char* args);
bool DebugCmdSet(char* args);
bool DebugCmdNext(char* args);
bool DebugCmdPeek(char* args);
bool DebugCmdCode(char* args);
bool DebugCmdWatch(char* args);
bool DebugCmdState(char* args);
bool DebugCmdSave(char* args);
bool DebugCmdPoke(char* args);
bool DebugCmdGoto(char* args);
bool DebugCmdFile(char* args);
bool DebugCmdEcho(char* args);
bool DebugCmdScript(char *args);

#endif
