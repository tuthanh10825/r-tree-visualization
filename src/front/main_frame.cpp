#include "main_frame.h"
#include <wx/wx.h>
main_frame::main_frame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1480, 800), wxDEFAULT_FRAME_STYLE & ~wxMAXIMIZE_BOX ^ wxRESIZE_BORDER)
{
	canvas = new drawing_canvas(this, wxID_ANY, wxDefaultPosition, wxDefaultSize); 
	wxPanel* setting_panel= new wxPanel(this, wxID_ANY); 
	setting_panel->SetBackgroundColour(wxColor(169, 169,169)); 
	clear_all = new wxButton(setting_panel, wxID_ANY, "Clear all"); 
	clear_all->Bind(wxEVT_BUTTON, &main_frame::on_clear_all, this);
	
	range_find = new wxButton(setting_panel, wxID_ANY, "Choose");
	range_find->Bind(wxEVT_BUTTON, &main_frame::on_choose, this); 
	wxBoxSizer* setting_panel_sizer = new wxBoxSizer(wxHORIZONTAL); 
	

	setting_panel_sizer->Add(clear_all, 0, wxALL, 5); 
	setting_panel_sizer->Add(range_find, 0, wxALL, 5); 

	setting_panel->SetSizerAndFit(setting_panel_sizer); 
	
	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL); 
	main_sizer->Add(canvas, 1, wxEXPAND);
	main_sizer->Add(setting_panel, 0, wxEXPAND); 
	this->SetSizer(main_sizer); 
}

void main_frame::on_clear_all(wxCommandEvent&)
{
	this->canvas->clear(); 
}

void main_frame::on_choose(wxCommandEvent&)
{
	range_find->SetLabelText(wxString("Stop")); 

	canvas->Unbind(wxEVT_LEFT_DOWN, &drawing_canvas::on_button_down, canvas);
	canvas->Unbind(wxEVT_MOTION, &drawing_canvas::on_button_move, canvas);
	canvas->Unbind(wxEVT_LEFT_UP, &drawing_canvas::on_button_up, canvas);
	canvas->Unbind(wxEVT_LEAVE_WINDOW, &drawing_canvas::on_mouse_leave, canvas);

	canvas->Bind(wxEVT_LEFT_DOWN, &drawing_canvas::choosing_rect_down, canvas);
	canvas->Bind(wxEVT_MOTION, &drawing_canvas::choosing_rect_move, canvas);
	canvas->Bind(wxEVT_LEFT_UP, &drawing_canvas::choosing_rect_up, canvas);
	canvas->Bind(wxEVT_LEAVE_WINDOW, &drawing_canvas::choosing_rect_leave, canvas); 
	range_find->Bind(wxEVT_BUTTON, &main_frame::on_stop_choose, this); 
}

void main_frame::on_stop_choose(wxCommandEvent&)
{
	
	range_find->SetLabelText("Choose"); 
	canvas->Unbind(wxEVT_LEFT_DOWN, &drawing_canvas::choosing_rect_down, canvas);
	canvas->Unbind(wxEVT_MOTION, &drawing_canvas::choosing_rect_move, canvas);
	canvas->Unbind(wxEVT_LEFT_UP, &drawing_canvas::choosing_rect_up, canvas);
	canvas->Unbind(wxEVT_LEAVE_WINDOW, &drawing_canvas::choosing_rect_leave, canvas);

	canvas->Bind(wxEVT_LEFT_DOWN, &drawing_canvas::on_button_down, canvas);
	canvas->Bind(wxEVT_MOTION, &drawing_canvas::on_button_move, canvas);
	canvas->Bind(wxEVT_LEFT_UP, &drawing_canvas::on_button_up, canvas);
	canvas->Bind(wxEVT_LEAVE_WINDOW, &drawing_canvas::on_mouse_leave, canvas);

	canvas->choosing_rect.clear();
	canvas->choosing_obj.clear();
	canvas->Refresh(); 

	range_find->Bind(wxEVT_BUTTON, &main_frame::on_choose, this);
}
