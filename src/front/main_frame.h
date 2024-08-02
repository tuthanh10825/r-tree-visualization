#ifndef MAIN_FRAME
#define MAIN_FRAME
#include "drawing_canvas.h"
#include <wx/wx.h>
class main_frame : public wxFrame
{
public: 
	main_frame(const wxString& title); 
	wxButton* clear_all;
	drawing_canvas* canvas;
	wxButton* range_find; 
private:
	void on_clear_all(wxCommandEvent&);
	void on_choose(wxCommandEvent&); 
	void on_stop_choose(wxCommandEvent&); 

};
#endif