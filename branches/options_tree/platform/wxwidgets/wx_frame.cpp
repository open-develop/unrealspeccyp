/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2013 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../platform.h"

#ifdef USE_WXWIDGETS

#include "../../tools/options.h"
#include "../../options_common.h"
#include "wx_cmdline.h"

#include <wx/wx.h>
#include <wx/dnd.h>
#include <wx/aboutdlg.h>

OPTION_USING(eOptionInt, op_drive);

namespace xPlatform
{

void InitSound();
void DoneSound();

wxWindow* CreateGLCanvas(wxWindow* parent);

OPTION_USING(eOptionBool, op_true_speed);

static struct eOptionWindowSize : public xOptions::eOptionInt
{
	eOptionWindowSize() { customizable = false; Set(1); }
	virtual const char* Name() const { return "window size"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "100%", "200%", "300%", NULL };
		return values;
	}
} op_window_size;
DECLARE_OPTION_ACCESSOR(eOptionInt, op_window_size);

static struct eOptionFullScreen : public xOptions::eOptionBool
{
	eOptionFullScreen() { customizable = false; }
	virtual const char* Name() const { return "full screen"; }
} op_full_screen;
DECLARE_OPTION_ACCESSOR(eOptionBool, op_full_screen);

extern const wxEventType evtMouseCapture;
extern const wxEventType evtSetStatusText;
extern const wxEventType evtExitFullScreen;

#ifndef _MAC
struct DropFilesTarget : public wxFileDropTarget
{
	virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
	{
		if(filenames.empty())
			return false;
		return Handler()->OnOpenFile(wxConvertWX2MB(filenames[0].c_str()));
	}
};
#endif//_MAC


//=============================================================================
//	Frame
//-----------------------------------------------------------------------------
class Frame : public wxFrame
{
public:
	Frame(const wxString& title, const wxPoint& pos, const eCmdLine& cmdline);
	void ShowFullScreen(bool on);

private:
	void OnReset(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event)	{ Close(true); };
	void OnAbout(wxCommandEvent& event);
	void OnOpenFile(wxCommandEvent& event);
	void OnSaveFile(wxCommandEvent& event);
	void SetFullScreen(bool on);
	void OnExitFullScreen(wxCommandEvent& event);
	void OnFullScreenToggle(wxCommandEvent& event);
	void OnResize(wxCommandEvent& event);
	void OnViewMode(wxCommandEvent& event);
	void OnViewFilteringToggle(wxCommandEvent& event);
	void OnTapeToggle(wxCommandEvent& event);
	void OnTapeFastToggle(wxCommandEvent& event);
	void OnBetaDiskDrive(wxCommandEvent& event);
	void OnJoy(wxCommandEvent& event);
	void OnPauseToggle(wxCommandEvent& event);
	void OnTrueSpeedToggle(wxCommandEvent& event);
	void OnResetToServiceRomToggle(wxCommandEvent& event);
	void OnAutoPlayImageToggle(wxCommandEvent& event);
	void OnMouseCapture(wxCommandEvent& event);
	void OnSetStatusText(wxCommandEvent& event);
	void OnQuickLoad(wxCommandEvent& event);
	void OnQuickSave(wxCommandEvent& event);
	void UpdateBetaDiskMenu();
	void UpdateJoyMenu();
	void UpdateViewZoomMenu();
	bool UpdateBoolOption(wxMenuItem* o, xOptions::eOptionBool* op, bool toggle = false) const; // returns option value

	enum
	{
		ID_Reset = 1, ID_ResetToServiceRomToggle, ID_Size200, ID_Size300,
		ID_ViewFillScreen, ID_ViewSmallBorder, ID_ViewNoBorder, ID_ViewFilteringToggle, ID_FullScreenToggle,
		ID_TapeToggle, ID_TapeFastToggle, ID_AutoPlayImageToggle,
		ID_JoyCursor, ID_JoyKempston, ID_JoyQAOP, ID_JoySinclair2,
		ID_PauseToggle, ID_TrueSpeedToggle,
		ID_BetaDiskDriveA, ID_BetaDiskDriveB, ID_BetaDiskDriveC, ID_BetaDiskDriveD,
		ID_QuickSave, ID_QuickLoad,
	};
	struct eJoyMenuItems
	{
		wxMenuItem* kempston;
		wxMenuItem* cursor;
		wxMenuItem* qaop;
		wxMenuItem* sinclair2;
	};
	struct eViewMenuItems
	{
		wxMenuItem* fill_screen;
		wxMenuItem* small_border;
		wxMenuItem* no_border;
		wxMenuItem* filtering;
	};
	eJoyMenuItems menu_joy;
	eViewMenuItems menu_view;
	wxMenuItem* menu_beta_disk_drive[4];
	wxMenuItem* menu_pause;
	wxMenuItem* menu_true_speed;
	wxMenuItem* menu_tape_fast;
	wxMenuItem* menu_reset_to_service_rom;
	wxMenuItem* menu_auto_play_image;
	wxMenuItem* menu_quick_save;

private:
	DECLARE_EVENT_TABLE()

	wxWindow*	gl_canvas;
	const wxSize org_size;
};

//=============================================================================
//	EVENT_TABLE
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(Frame, wxFrame)
	EVT_MENU(wxID_EXIT,				Frame::OnQuit)
	EVT_MENU(wxID_ABOUT,			Frame::OnAbout)
	EVT_MENU(wxID_OPEN,				Frame::OnOpenFile)
	EVT_MENU(wxID_SAVE,				Frame::OnSaveFile)
	EVT_MENU(Frame::ID_Reset,		Frame::OnReset)
	EVT_MENU(wxID_ZOOM_100,			Frame::OnResize)
	EVT_MENU(Frame::ID_Size200,		Frame::OnResize)
	EVT_MENU(Frame::ID_Size300,		Frame::OnResize)
	EVT_MENU(Frame::ID_ViewFillScreen, Frame::OnViewMode)
	EVT_MENU(Frame::ID_ViewSmallBorder, Frame::OnViewMode)
	EVT_MENU(Frame::ID_ViewNoBorder, Frame::OnViewMode)
	EVT_MENU(Frame::ID_ViewFilteringToggle, Frame::OnViewFilteringToggle)
	EVT_MENU(Frame::ID_FullScreenToggle, Frame::OnFullScreenToggle)
	EVT_MENU(Frame::ID_TapeToggle,	Frame::OnTapeToggle)
	EVT_MENU(Frame::ID_TapeFastToggle,Frame::OnTapeFastToggle)
	EVT_MENU(Frame::ID_BetaDiskDriveA, Frame::OnBetaDiskDrive)
	EVT_MENU(Frame::ID_BetaDiskDriveB, Frame::OnBetaDiskDrive)
	EVT_MENU(Frame::ID_BetaDiskDriveC, Frame::OnBetaDiskDrive)
	EVT_MENU(Frame::ID_BetaDiskDriveD, Frame::OnBetaDiskDrive)
	EVT_MENU(Frame::ID_JoyKempston,	Frame::OnJoy)
	EVT_MENU(Frame::ID_JoyCursor,	Frame::OnJoy)
	EVT_MENU(Frame::ID_JoyQAOP,		Frame::OnJoy)
	EVT_MENU(Frame::ID_JoySinclair2,Frame::OnJoy)
	EVT_MENU(Frame::ID_PauseToggle,	Frame::OnPauseToggle)
	EVT_MENU(Frame::ID_TrueSpeedToggle,	Frame::OnTrueSpeedToggle)
	EVT_MENU(Frame::ID_ResetToServiceRomToggle,	Frame::OnResetToServiceRomToggle)
	EVT_MENU(Frame::ID_AutoPlayImageToggle,	Frame::OnAutoPlayImageToggle)
	EVT_MENU(Frame::ID_QuickLoad,	Frame::OnQuickLoad)
	EVT_MENU(Frame::ID_QuickSave,	Frame::OnQuickSave)
	EVT_COMMAND(wxID_ANY, evtMouseCapture, Frame::OnMouseCapture)
	EVT_COMMAND(wxID_ANY, evtSetStatusText, Frame::OnSetStatusText)
	EVT_COMMAND(wxID_ANY, evtExitFullScreen, Frame::OnExitFullScreen)
END_EVENT_TABLE()


//=============================================================================
//	Frame::Frame
//-----------------------------------------------------------------------------
Frame::Frame(const wxString& title, const wxPoint& pos, const eCmdLine& cmdline)
	: wxFrame((wxFrame*)NULL, -1, title, pos)
	, org_size(320, 240)
{
#ifdef _WINDOWS
	SetIcon(wxICON(unreal_speccy_portable));
#endif//_WINDOWS
#ifdef _LINUX
	SetIcon(wxIcon(wxT("unreal_speccy_portable.xpm")));
#endif//_LINUX
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(wxID_OPEN, _("&Open...\tF3"));
	menuFile->Append(wxID_SAVE, _("&Save...\tF2"));

	menuFile->AppendSeparator();
	menuFile->Append(ID_QuickLoad, _("Quick &Load\tF4"));
	menu_quick_save = menuFile->Append(ID_QuickSave, _("&Quick Save\tF6"));
	menu_quick_save->Enable(false);

#ifdef _MAC
	menuFile->Append(wxID_ABOUT, _("About ") + title);
#else//_MAC
	SetDropTarget(new DropFilesTarget);
	menuFile->AppendSeparator();
#endif//_MAC
	menu_auto_play_image = menuFile->Append(ID_AutoPlayImageToggle, _("&Auto launch programs"), _(""), wxITEM_CHECK);
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT, _("E&xit"));

	wxMenu* menuDevice = new wxMenu;
	menuDevice->Append(ID_TapeToggle, _("&Start/Stop tape\tF5"));
	menu_tape_fast = menuDevice->Append(ID_TapeFastToggle, _("Tape &fast"), _(""), wxITEM_CHECK);

	wxMenu* menuBetaDisk = new wxMenu;
	menu_beta_disk_drive[0] = menuBetaDisk->Append(ID_BetaDiskDriveA, _("&A"), _(""), wxITEM_CHECK);
	menu_beta_disk_drive[1] = menuBetaDisk->Append(ID_BetaDiskDriveB, _("&B"), _(""), wxITEM_CHECK);
	menu_beta_disk_drive[2] = menuBetaDisk->Append(ID_BetaDiskDriveC, _("&C"), _(""), wxITEM_CHECK);
	menu_beta_disk_drive[3] = menuBetaDisk->Append(ID_BetaDiskDriveD, _("&D"), _(""), wxITEM_CHECK);
	menuDevice->Append(-1, _("Beta disk &drive"), menuBetaDisk);

	menu_pause = menuDevice->Append(ID_PauseToggle, _("&Pause\tF7"), _(""), wxITEM_CHECK);
	menu_true_speed = menuDevice->Append(ID_TrueSpeedToggle, _("&True speed\tF8"), _(""), wxITEM_CHECK);
	menu_reset_to_service_rom = menuDevice->Append(ID_ResetToServiceRomToggle, _("Reset to service R&OM"), _(""), wxITEM_CHECK);
	menuDevice->Append(ID_Reset, _("&Reset\tF12"));

	wxMenu* menuJoy = new wxMenu;
	menu_joy.cursor = menuJoy->Append(ID_JoyCursor, _("&Cursor"), _(""), wxITEM_CHECK);
	menu_joy.kempston = menuJoy->Append(ID_JoyKempston, _("&Kempston"), _(""), wxITEM_CHECK);
	menu_joy.qaop = menuJoy->Append(ID_JoyQAOP, _("&QAOP"), _(""), wxITEM_CHECK);
	menu_joy.sinclair2 = menuJoy->Append(ID_JoySinclair2, _("&Sinclair 2"), _(""), wxITEM_CHECK);
	menuDevice->Append(-1, _("&Joystick"), menuJoy);

	wxMenu* menuWindow = new wxMenu;
	menuWindow->Append(wxID_ZOOM_100, _("Size &100%\tCtrl+1"));
	menuWindow->Append(ID_Size200, _("Size &200%\tCtrl+2"));
	menuWindow->Append(ID_Size300, _("Size &300%\tCtrl+3"));

	wxMenu* menuView = new wxMenu;
	menu_view.fill_screen = menuView->Append(ID_ViewFillScreen, _("Fill screen\tCtrl+Shift+1"), _(""), wxITEM_CHECK);
	menu_view.small_border = menuView->Append(ID_ViewSmallBorder, _("Small border\tCtrl+Shift+2"), _(""), wxITEM_CHECK);
	menu_view.no_border = menuView->Append(ID_ViewNoBorder, _("No border\tCtrl+Shift+3"), _(""), wxITEM_CHECK);
	menu_view.filtering = menuView->Append(ID_ViewFilteringToggle, _("Filtering\tCtrl+Shift+F"), _(""), wxITEM_CHECK);
	menuView->Append(ID_FullScreenToggle, _("&Full screen\tCtrl+F"));

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, _("File"));
	menuBar->Append(menuView, _("View"));
	menuBar->Append(menuDevice, _("Device"));
	menuBar->Append(menuWindow, _("Window"));

#ifndef _MAC
	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT, _("&About ") + title);
	menuBar->Append(menuHelp, _("Help"));
#endif//_MAC

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText(_("Ready..."));

	SetClientSize(org_size);
	SetMinSize(GetSize());

	if(cmdline.size_percent >= 0)
	{
		op_full_screen.Set(false);
		SetClientSize(org_size*cmdline.size_percent/100);
	}
	else
	{
		SetClientSize(org_size*(op_window_size + 1));
	}

	gl_canvas = CreateGLCanvas(this);
	gl_canvas->SetFocus();

	UpdateBetaDiskMenu();
	xOptions::eOptionBool* op_true_speed = OPTION_GET(op_true_speed);
	if(cmdline.true_speed != eCmdLine::V_DEFAULT && op_true_speed)
	{
		op_true_speed->Set(cmdline.true_speed == eCmdLine::V_ON);
		op_true_speed->Apply();
	}
	if(cmdline.full_screen != eCmdLine::V_DEFAULT)
		op_full_screen.Set(cmdline.full_screen == eCmdLine::V_ON);
	if(!cmdline.joystick.empty())
	{
		xOptions::eOptionInt* op_joy = OPTION_GET(op_joy);
		SAFE_CALL(op_joy)->Value(wxConvertWX2MB(cmdline.joystick));
	}
	UpdateJoyMenu();
	menu_true_speed->Check(op_true_speed && *op_true_speed);

	UpdateBoolOption(menu_tape_fast, OPTION_GET(op_tape_fast));
	UpdateBoolOption(menu_reset_to_service_rom, OPTION_GET(op_reset_to_service_rom));
	UpdateBoolOption(menu_auto_play_image, OPTION_GET(op_auto_play_image));
	UpdateBoolOption(menu_view.filtering, OPTION_GET(op_filtering));
	UpdateViewZoomMenu();

	if(!cmdline.file_to_open.empty())
		Handler()->OnOpenFile(wxConvertWX2MB(cmdline.file_to_open));
}
//=============================================================================
//	Frame::ShowFullScreen
//-----------------------------------------------------------------------------
void Frame::ShowFullScreen(bool on)
{
#ifdef _MAC
	if(on)
	{
		GetStatusBar()->Hide();
	}
	else
	{
		GetStatusBar()->Show();
	}
#endif//_MAC
	wxFrame::ShowFullScreen(on, wxFULLSCREEN_ALL);
}
//=============================================================================
//	Frame::UpdateBoolOption
//-----------------------------------------------------------------------------
bool Frame::UpdateBoolOption(wxMenuItem* o, xOptions::eOptionBool* op, bool toggle) const
{
	if(op && toggle)
		op->Change();
	bool on = op && *op;
	o->Check(on);
	return on;
}
//=============================================================================
//	Frame::OnReset
//-----------------------------------------------------------------------------
void Frame::OnReset(wxCommandEvent& event)
{
	if(Handler()->OnAction(A_RESET) == AR_OK)
	{
		SetStatusText(_("Reset OK"));
		menu_quick_save->Enable(false);
	}
	else
		SetStatusText(_("Reset FAILED"));
}
//=============================================================================
//	Frame::OnAbout
//-----------------------------------------------------------------------------
void Frame::OnAbout(wxCommandEvent& event)
{
	wxAboutDialogInfo info;
	info.SetName(GetTitle());
	info.SetDescription(_("Portable ZX Spectrum emulator."));
	info.SetCopyright(_("Copyright (C) 2001-2013 SMT, Dexus, Alone Coder, deathsoft, djdron, scor."));
#ifndef _MAC
	info.SetVersion(_("0.0.56"));
	info.SetWebSite(_("http://code.google.com/p/unrealspeccyp/"));
	info.SetLicense(_(
"This program is free software: you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation, either version 3 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"
			));
#endif//_MAC
	wxAboutBox(info);
}
//=============================================================================
//	Frame::OnOpenFile
//-----------------------------------------------------------------------------
void Frame::OnOpenFile(wxCommandEvent& event)
{
	wxFileDialog fd(this, wxFileSelectorPromptStr, wxConvertMB2WX(OpLastFolder()));
	fd.SetWildcard(
			L"Supported files|*.sna;*.z80;*.szx;*.rzx;*.trd;*.scl;*.fdi;*.tap;*.csw;*.tzx;*.zip;"
							L"*.SNA;*.Z80;*.SZX;*.RZX;*.TRD;*.SCL;*.FDI;*.TAP;*.CSW;*.TZX;*.ZIP|"
			L"All files|*.*|"
			L"Snapshot files (*.sna;*.z80;*.szx)|*.sna;*.z80;*.szx;*.SNA;*.Z80;*.SZX|"
			L"Replay files (*.rzx)|*.rzx;*.RZX|"
			L"Disk images (*.trd;*.scl;*.fdi)|*.trd;*.scl;*.fdi;*.TRD;*.SCL;*.FDI|"
			L"Tape files (*.tap;*.csw;*.tzx)|*.tap;*.csw;*.tzx;*.TAP;*.CSW;*.TZX|"
			L"ZIP archives (*.zip)|*.zip;*.ZIP"
		);
	if(fd.ShowModal() == wxID_OK)
	{
		if(Handler()->OnOpenFile(wxConvertWX2MB(fd.GetPath().c_str())))
		{
			SetStatusText(_("File open OK"));
			menu_quick_save->Enable(true);
		}
		else
			SetStatusText(_("File open FAILED"));
	}
}
//=============================================================================
//	Frame::OnSaveFile
//-----------------------------------------------------------------------------
void Frame::OnSaveFile(wxCommandEvent& event)
{
	Handler()->VideoPaused(true);
	wxFileDialog fd(this, wxFileSelectorPromptStr, wxConvertMB2WX(OpLastFolder()), wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	fd.SetWildcard(
			L"Snapshot files (*.sna)|*.sna;*.SNA|"
			L"Screenshot files (*.png)|*.png;*.PNG"
		);
	if(fd.ShowModal() == wxID_OK)
	{
		wxString path = fd.GetPath();
		int fi = fd.GetFilterIndex();
		size_t p = path.length() - 4;
		if(path.length() < 4 || (
				path.rfind(L".sna") != p && path.rfind(L".SNA") != p &&
				path.rfind(L".png") != p && path.rfind(L".PNG") != p))
			path += fi ? L".png" : L".sna";
		if(Handler()->OnSaveFile(wxConvertWX2MB(path.c_str())))
			SetStatusText(_("File save OK"));
		else
			SetStatusText(_("File save FAILED"));
	}
	Handler()->VideoPaused(false);
}
//=============================================================================
//	Frame::SetFullScreen
//-----------------------------------------------------------------------------
void Frame::SetFullScreen(bool on)
{
	op_full_screen.Set(on);
	if(IsFullScreen() != op_full_screen)
	{
		ShowFullScreen(op_full_screen);
	}
}
//=============================================================================
//	Frame::OnExitFullScreen
//-----------------------------------------------------------------------------
void Frame::OnExitFullScreen(wxCommandEvent& event)
{
	SetFullScreen(false);
}
//=============================================================================
//	Frame::OnToggleFullScreen
//-----------------------------------------------------------------------------
void Frame::OnFullScreenToggle(wxCommandEvent& event)
{
	SetFullScreen(!op_full_screen);
}
//=============================================================================
//	Frame::OnResize
//-----------------------------------------------------------------------------
void Frame::OnResize(wxCommandEvent& event)
{
	switch(event.GetId())
	{
	case wxID_ZOOM_100:	op_window_size.Set(0);	break;
	case ID_Size200:	op_window_size.Set(1);	break;
	case ID_Size300:	op_window_size.Set(2);	break;
	}
	if(IsFullScreen())
	{
		ShowFullScreen(false);
		op_full_screen.Set(false);
	}
	if(IsMaximized())
		Maximize(false);
	SetClientSize(org_size*(op_window_size + 1));
}
//=============================================================================
//	Frame::OnViewMode
//-----------------------------------------------------------------------------
void Frame::OnViewMode(wxCommandEvent& event)
{
	xOptions::eOptionInt* op_zoom = OPTION_GET(op_zoom);
	switch(event.GetId())
	{
	case ID_ViewFillScreen:		op_zoom->Set(0);		break;
	case ID_ViewSmallBorder:	op_zoom->Set(1);		break;
	case ID_ViewNoBorder:		op_zoom->Set(2);		break;
	}
	UpdateViewZoomMenu();
}
//=============================================================================
//	Frame::OnTapeToggle
//-----------------------------------------------------------------------------
void Frame::OnTapeToggle(wxCommandEvent& event)
{
	switch(Handler()->OnAction(A_TAPE_TOGGLE))
	{
	case AR_TAPE_STARTED:		SetStatusText(_("Tape started"));	break;
	case AR_TAPE_STOPPED:		SetStatusText(_("Tape stopped"));	break;
	case AR_TAPE_NOT_INSERTED:	SetStatusText(_("Tape not inserted"));	break;
	default: break;
	}
}
//=============================================================================
//	Frame::OnTapeFastToggle
//-----------------------------------------------------------------------------
void Frame::OnTapeFastToggle(wxCommandEvent& event)
{
	xOptions::eOptionBool* op_tape_fast = OPTION_GET(op_tape_fast);
	SAFE_CALL(op_tape_fast)->Change();
	bool tape_fast = op_tape_fast && *op_tape_fast;
	menu_tape_fast->Check(tape_fast);
	SetStatusText(tape_fast ? _("Fast tape on") : _("Fast tape off"));
}
//=============================================================================
//	Frame::OnBetaDiskDrive
//-----------------------------------------------------------------------------
void Frame::OnBetaDiskDrive(wxCommandEvent& event)
{
	xOptions::eOptionInt* op_drive = OPTION_GET(op_drive);
	switch(event.GetId())
	{
	case ID_BetaDiskDriveA: op_drive->Set(0); SetStatusText(_("Drive A selected"));	break;
	case ID_BetaDiskDriveB: op_drive->Set(1); SetStatusText(_("Drive B selected"));	break;
	case ID_BetaDiskDriveC: op_drive->Set(2); SetStatusText(_("Drive C selected"));	break;
	case ID_BetaDiskDriveD: op_drive->Set(3); SetStatusText(_("Drive D selected"));	break;
	}
	UpdateBetaDiskMenu();
}
//=============================================================================
//	Frame::OnJoy
//-----------------------------------------------------------------------------
void Frame::OnJoy(wxCommandEvent& event)
{
	xOptions::eOptionInt* op_joy = OPTION_GET(op_joy);
	switch(event.GetId())
	{
	case ID_JoyKempston:	op_joy->Set(J_KEMPSTON);	SetStatusText(_("Kempston selected"));	break;
	case ID_JoyCursor:		op_joy->Set(J_CURSOR);		SetStatusText(_("Cursor selected"));	break;
	case ID_JoyQAOP:		op_joy->Set(J_QAOP);		SetStatusText(_("QAOP selected"));		break;
	case ID_JoySinclair2:	op_joy->Set(J_SINCLAIR2);	SetStatusText(_("Sinclair 2 selected"));break;
	}
	UpdateJoyMenu();
}
//=============================================================================
//	Frame::OnPauseToggle
//-----------------------------------------------------------------------------
void Frame::OnPauseToggle(wxCommandEvent& event)
{
	if(menu_pause->IsChecked())
	{
		Handler()->VideoPaused(true);
		SetStatusText(_("Paused..."));
	}
	else
	{
		Handler()->VideoPaused(false);
		SetStatusText(_("Ready..."));
	}
}
//=============================================================================
//	Frame::OnViewFilteringToggle
//-----------------------------------------------------------------------------
void Frame::OnViewFilteringToggle(wxCommandEvent& event)
{
	if(UpdateBoolOption(menu_view.filtering, OPTION_GET(op_filtering), true))
		SetStatusText(_("Filtering on"));
	else
		SetStatusText(_("Filtering off"));
}
//=============================================================================
//	Frame::OnTrueSpeedToggle
//-----------------------------------------------------------------------------
void Frame::OnTrueSpeedToggle(wxCommandEvent& event)
{
	if(UpdateBoolOption(menu_true_speed, OPTION_GET(op_true_speed), true))
		SetStatusText(_("True speed (50Hz mode) on"));
	else
		SetStatusText(_("True speed off"));
}
//=============================================================================
//	Frame::OnResetToServiceRomToggle
//-----------------------------------------------------------------------------
void Frame::OnResetToServiceRomToggle(wxCommandEvent& event)
{
	if(UpdateBoolOption(menu_reset_to_service_rom, OPTION_GET(op_reset_to_service_rom), true))
		SetStatusText(_("Reset to service ROM"));
	else
		SetStatusText(_("Reset to usual ROM"));
}
//=============================================================================
//	Frame::OnResetToServiceRomToggle
//-----------------------------------------------------------------------------
void Frame::OnAutoPlayImageToggle(wxCommandEvent& event)
{
	if(UpdateBoolOption(menu_auto_play_image, OPTION_GET(op_auto_play_image), true))
		SetStatusText(_("Auto launch on"));
	else
		SetStatusText(_("Auto launch off"));
}
//=============================================================================
//	Frame::OnMouseCapture
//-----------------------------------------------------------------------------
void Frame::OnMouseCapture(wxCommandEvent& event)
{
	SetStatusText(event.GetId() ? _("Mouse captured, press ESC to cancel") : _("Mouse released"));
}
//=============================================================================
//	Frame::OnSetStatusText
//-----------------------------------------------------------------------------
void Frame::OnSetStatusText(wxCommandEvent& event)
{
	if(event.GetString() == L"rzx_finished")
		SetStatusText(_("RZX playback finished"));
	else if(event.GetString() == L"rzx_sync_lost")
		SetStatusText(_("RZX error - sync lost"));
	else if(event.GetString() == L"rzx_invalid")
		SetStatusText(_("RZX error - invalid data"));
	else if(event.GetString() == L"rzx_unsupported")
		SetStatusText(_("RZX error - unsupported format"));
}
//=============================================================================
//	Frame::OnQuickLoad
//-----------------------------------------------------------------------------
void Frame::OnQuickLoad(wxCommandEvent& event)
{
	xOptions::eOptionBool* o = OPTION_GET(op_load_state);
	if(o)
	{
		o->Change();
		SetStatusText(*o ? _("Quick load OK") : _("Quick load FAILED"));
		if(*o)
			menu_quick_save->Enable(true);
	}
}
//=============================================================================
//	Frame::OnQuickSave
//-----------------------------------------------------------------------------
void Frame::OnQuickSave(wxCommandEvent& event)
{
	xOptions::eOptionBool* o = OPTION_GET(op_save_state);
	if(o)
	{
		o->Change();
		SetStatusText(*o ? _("Quick save OK") : _("Quick save FAILED"));
	}
}
//=============================================================================
//	Frame::UpdateBetaDiskMenu
//-----------------------------------------------------------------------------
void Frame::UpdateBetaDiskMenu()
{
	xOptions::eOptionInt* op_drive = OPTION_GET(op_drive);
	menu_beta_disk_drive[0]->Check(*op_drive == 0);
	menu_beta_disk_drive[1]->Check(*op_drive == 1);
	menu_beta_disk_drive[2]->Check(*op_drive == 2);
	menu_beta_disk_drive[3]->Check(*op_drive == 3);
}
//=============================================================================
//	Frame::UpdateJoyMenu
//-----------------------------------------------------------------------------
void Frame::UpdateJoyMenu()
{
	xOptions::eOptionInt* op_joy = OPTION_GET(op_joy);
	menu_joy.kempston->Check(*op_joy == J_KEMPSTON);
	menu_joy.cursor->Check(*op_joy == J_CURSOR);
	menu_joy.qaop->Check(*op_joy == J_QAOP);
	menu_joy.sinclair2->Check(*op_joy == J_SINCLAIR2);
}
//=============================================================================
//	Frame::UpdateViewZoomMenu
//-----------------------------------------------------------------------------
void Frame::UpdateViewZoomMenu()
{
	xOptions::eOptionInt* op_zoom  = OPTION_GET(op_zoom );
	menu_view.fill_screen->Check(*op_zoom == 0);
	menu_view.small_border->Check(*op_zoom == 1);
	menu_view.no_border->Check(*op_zoom == 2);
}

//=============================================================================
//	CreateFrame
//-----------------------------------------------------------------------------
wxWindow* CreateFrame(const wxString& title, const wxPoint& pos, const eCmdLine& cmdline)
{
	Frame* frame = new Frame(title, pos, cmdline);
	frame->Show(true);
	if(op_full_screen)
		frame->ShowFullScreen(true);
	return frame;
}

}
//namespace xPlatform

#endif//USE_WXWIDGETS
