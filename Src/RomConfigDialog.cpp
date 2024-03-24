/****************************************************************
BeebEm - BBC Micro and Master 128 Emulator
Copyright (C) 2009  Mike Wyatt

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

// BeebWin ROM Configuration Dialog

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <stdio.h>

#include "RomConfigDialog.h"
#include "BeebWin.h"
#include "BeebMem.h"
#include "FileDialog.h"
#include "ListView.h"
#include "Main.h"
#include "Resource.h"
#include "SysVia.h"

static const char* szModel[] = { "BBC B", "Integra-B", "B Plus", "Master 128" };
static char szDefaultROMPath[MAX_PATH] = {0};
static char szDefaultROMConfigPath[MAX_PATH] = {0};

static bool WriteROMConfigFile(const char *filename, ROMConfigFile RomConfig);

/****************************************************************************/

RomConfigDialog::RomConfigDialog(HINSTANCE hInstance,
                                 HWND hwndParent,
                                 ROMConfigFile Config) :
	Dialog(hInstance, hwndParent, IDD_ROMCONFIG),
	m_hWndROMList(nullptr),
	m_hWndModel(nullptr),
	m_Model(MachineType)
{
	memcpy(m_RomConfig, Config, sizeof(ROMConfigFile));
}

/****************************************************************************/

const ROMConfigFile* RomConfigDialog::GetRomConfig() const
{
	return &m_RomConfig;
}

/****************************************************************************/

void RomConfigDialog::UpdateROMField(int Row)
{
	char szROMFile[_MAX_PATH];
	bool Unplugged = false;
	int Bank;

	if (m_Model == Model::Master128)
	{
		Bank = 16 - Row;

		if (Bank >= 0 && Bank <= 7)
		{
			Unplugged = (CMOSRAM[20] & (1 << Bank)) != 0;
		}
		else if (Bank >= 8 && Bank <= 15)
		{
			Unplugged = (CMOSRAM[21] & (1 << (Bank - 8))) != 0;
		}
	}

	strncpy(szROMFile, m_RomConfig[static_cast<int>(m_Model)][Row], _MAX_PATH);

	if (Unplugged)
	{
		strncat(szROMFile, " (unplugged)", _MAX_PATH);
	}

	LVSetItemText(m_hWndROMList, Row, 1, szROMFile);
}

/****************************************************************************/

void RomConfigDialog::FillROMList()
{
	Edit_SetText(m_hWndModel, szModel[static_cast<int>(m_Model)]);

	ListView_DeleteAllItems(m_hWndROMList);

	int Row = 0;
	LVInsertItem(m_hWndROMList, Row, 0, "OS", 16);
	LVSetItemText(m_hWndROMList, Row, 1, m_RomConfig[static_cast<int>(m_Model)][0]);

	for (Row = 1; Row <= 16; ++Row)
	{
		int Bank = 16 - Row;

		char str[20];
		sprintf(str, "%02d (%X)", Bank, Bank);

		LVInsertItem(m_hWndROMList, Row, 0, str, Bank);
		UpdateROMField(Row);
	}
}

/****************************************************************************/

INT_PTR RomConfigDialog::DlgProc(UINT   nMessage,
                                 WPARAM wParam,
                                 LPARAM /* lParam */)
{
	switch (nMessage)
	{
		case WM_INITDIALOG:
			m_hWndModel = GetDlgItem(m_hwnd, IDC_MODEL);
			m_hWndROMList = GetDlgItem(m_hwnd, IDC_ROMLIST);

			ListView_SetExtendedListViewStyle(m_hWndROMList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			LVInsertColumn(m_hWndROMList, 0, "Bank", LVCFMT_LEFT, 45);
			LVInsertColumn(m_hWndROMList, 1, "ROM File", LVCFMT_LEFT, 283);

			FillROMList();
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_BBCB:
				m_Model = Model::B;
				FillROMList();
				return TRUE;

			case IDC_INTEGRAB:
				m_Model = Model::IntegraB;
				FillROMList();
				return TRUE;

			case IDC_BBCBPLUS:
				m_Model = Model::BPlus;
				FillROMList();
				return TRUE;

			case IDC_MASTER128:
				m_Model = Model::Master128;
				FillROMList();
				return TRUE;

			case IDC_SELECTROM: {
				int Row = ListView_GetSelectionMark(m_hWndROMList);

				if (Row >= 0 && Row <= 16)
				{
					char szROMFile[MAX_PATH];
					szROMFile[0] = '\0';

					if (GetROMFile(szROMFile))
					{
						// Strip user data path
						char szROMPath[MAX_PATH];
						strcpy(szROMPath, "BeebFile");
						mainWin->GetDataPath(mainWin->GetUserDataPath(), szROMPath);

						int nROMPathLen = (int)strlen(szROMPath);

						if (strncmp(szROMFile, szROMPath, nROMPathLen) == 0)
						{
							strcpy(szROMFile, szROMFile + nROMPathLen + 1);
						}

						strcpy(m_RomConfig[static_cast<int>(m_Model)][Row], szROMFile);
						UpdateROMField(Row);
					}
				}

				LVSetFocus(m_hWndROMList);
				break;
			}

			case IDC_MARKWRITABLE: {
				int Row = ListView_GetSelectionMark(m_hWndROMList);

				if (Row >= 1 && Row <= 16)
				{
					char *cfg = m_RomConfig[static_cast<int>(m_Model)][Row];

					if (strcmp(cfg, BANK_EMPTY) != 0 && strcmp(cfg, BANK_RAM) != 0)
					{
						if (strlen(cfg) > 4 && strcmp(cfg + strlen(cfg) - 4, ROM_WRITABLE) == 0)
							cfg[strlen(cfg) - 4] = 0;
						else
							strcat(cfg, ROM_WRITABLE);

						UpdateROMField(Row);
					}
				}

				LVSetFocus(m_hWndROMList);
				break;
			}

			case IDC_RAM: {
				int Row = ListView_GetSelectionMark(m_hWndROMList);

				if (Row >= 1 && Row <= 16)
				{
					strcpy(m_RomConfig[static_cast<int>(m_Model)][Row], BANK_RAM);
					UpdateROMField(Row);
				}

				LVSetFocus(m_hWndROMList);
				break;
			}

			case IDC_EMPTY: {
				int Row = ListView_GetSelectionMark(m_hWndROMList);

				if (Row >= 1 && Row <= 16)
				{
					strcpy(m_RomConfig[static_cast<int>(m_Model)][Row], BANK_EMPTY);
					UpdateROMField(Row);
				}

				LVSetFocus(m_hWndROMList);
				break;
			}

			case IDC_SAVE:
				SaveROMConfigFile();
				LVSetFocus(m_hWndROMList);
				break;

			case IDC_LOAD:
				LoadROMConfigFile();
				LVSetFocus(m_hWndROMList);
				break;

			case IDOK:
				EndDialog(m_hwnd, TRUE);
				return TRUE;

			case IDCANCEL:
				EndDialog(m_hwnd, FALSE);
				return TRUE;
			}
			break;
	}

	return FALSE;
}

/****************************************************************************/

bool RomConfigDialog::LoadROMConfigFile()
{
	char DefaultPath[MAX_PATH];
	char szROMConfigPath[MAX_PATH];
	szROMConfigPath[0] = '\0';
	bool success = false;
	const char* filter = "ROM Config File (*.cfg)\0*.cfg\0";

	if (szDefaultROMConfigPath[0] != '\0')
	{
		strcpy(DefaultPath, szDefaultROMConfigPath);
	}
	else
	{
		strcpy(DefaultPath, mainWin->GetUserDataPath());
	}

	FileDialog fileDialog(m_hwnd, szROMConfigPath, MAX_PATH, DefaultPath, filter);

	if (fileDialog.Open())
	{
		// Save directory as default for next time
		unsigned int PathLength = (unsigned int)(strrchr(szROMConfigPath, '\\') - szROMConfigPath);
		strncpy(szDefaultROMConfigPath, szROMConfigPath, PathLength);
		szDefaultROMConfigPath[PathLength] = 0;

		// Read the file
		ROMConfigFile LoadedROMCfg;
		if (ReadROMConfigFile(szROMConfigPath, LoadedROMCfg))
		{
			// Copy in loaded config
			memcpy(&m_RomConfig, &LoadedROMCfg, sizeof(ROMConfigFile));
			FillROMList();
			success = true;
		}
	}

	return success;
}

/****************************************************************************/

bool RomConfigDialog::SaveROMConfigFile()
{
	char DefaultPath[MAX_PATH];
	char szROMConfigPath[MAX_PATH];
	szROMConfigPath[0] = '\0';
	bool success = false;
	const char* filter = "ROM Config File (*.cfg)\0*.cfg\0";

	if (szDefaultROMConfigPath[0] != '\0')
	{
		strcpy(DefaultPath, szDefaultROMConfigPath);
	}
	else
	{
		strcpy(DefaultPath, mainWin->GetUserDataPath());
	}

	FileDialog fileDialog(m_hwnd, szROMConfigPath, MAX_PATH, DefaultPath, filter);

	if (fileDialog.Save())
	{
		// Save directory as default for next time
		unsigned int PathLength = (unsigned int)(strrchr(szROMConfigPath, '\\') - szROMConfigPath);
		strncpy(szDefaultROMConfigPath, szROMConfigPath, PathLength);
		szDefaultROMConfigPath[PathLength] = 0;

		// Add a file extension if required
		if (strchr(szROMConfigPath, '.') == NULL)
		{
			strcat(szROMConfigPath, ".cfg");
		}

		// Save the file
		if (WriteROMConfigFile(szROMConfigPath, m_RomConfig))
		{
			success = true;
		}
	}

	return success;
}

/****************************************************************************/

static bool WriteROMConfigFile(const char *filename, ROMConfigFile ROMConfig)
{
	FILE *fd = fopen(filename, "w");
	if (!fd)
	{
		mainWin->Report(MessageType::Error,
		                "Failed to write ROM configuration file:\n  %s", filename);

		return false;
	}

	for (int Model = 0; Model < 4; ++Model)
	{
		for (int Bank = 0; Bank < 17; ++Bank)
		{
			fprintf(fd, "%s\n", ROMConfig[Model][Bank]);
		}
	}

	fclose(fd);

	return true;
}

/****************************************************************************/

bool RomConfigDialog::GetROMFile(char *pszFileName)
{
	char DefaultPath[MAX_PATH];
	char szROMPath[MAX_PATH];
	bool success = false;
	const char* filter = "ROM File (*.rom)\0*.rom\0";

	strcpy(szROMPath, "BeebFile");
	mainWin->GetDataPath(mainWin->GetUserDataPath(), szROMPath);

	if (szDefaultROMPath[0])
		strcpy(DefaultPath, szDefaultROMPath);
	else
		strcpy(DefaultPath, szROMPath);

	FileDialog fileDialog(m_hwnd, pszFileName, MAX_PATH, DefaultPath, filter);

	if (fileDialog.Open())
	{
		// Save directory as default for next time
		unsigned int PathLength = (unsigned int)(strrchr(pszFileName, '\\') - pszFileName);
		strncpy(szDefaultROMPath, pszFileName, PathLength);
		szDefaultROMPath[PathLength] = 0;

		success = true;
	}

	return success;
}