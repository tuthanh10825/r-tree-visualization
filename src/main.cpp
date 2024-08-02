// r-tree visualization.cpp : Defines the entry point for the application.
//
//#include <wx/wx.h>

#include "main_frame.h"


class myApp : public wxApp
{
	virtual bool OnInit()
	{


		main_frame* frame = new main_frame(wxString("R-Tree visualization")); 
		frame->Show(true);
		return true;
	}
};

	wxIMPLEMENT_APP(myApp);