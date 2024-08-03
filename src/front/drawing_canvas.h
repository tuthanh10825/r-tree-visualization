#ifndef DRAWING_CANVAS
#define DRAWING_CANVAS
#include <wx/wx.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include "rtree.h"
#include <set>
class drawing_canvas : public wxWindow
{
public: 
	drawing_canvas(wxWindow* parent, wxWindowID id, wxPoint position, wxSize Size); 
	virtual ~drawing_canvas() {}

	void on_paint(wxPaintEvent& evt); 
	void on_button_down(wxMouseEvent&); 
	void on_button_move(wxMouseEvent&);
	void on_button_up(wxMouseEvent&);
	void on_mouse_leave(wxMouseEvent&);

	void choosing_rect_down(wxMouseEvent& evt);
	void choosing_rect_move(wxMouseEvent& evt); 
	void choosing_rect_up(wxMouseEvent& evt);
	void choosing_rect_leave(wxMouseEvent&);

	int M = 4; 
	rtree tree = rtree(M); 

	std::vector<wxPoint2DDouble> current_object;
	std::vector<std::vector<wxPoint2DDouble>> mbrs; 

	std::vector<wxPoint2DDouble> choosing_rect; 
	vector<vector<wxPoint2DDouble>> choosing_obj; 
	bool is_drawing = false; 
	bool is_choosing = false;

	
	void clear();
public: 
	void draw_tree(rtree::TreeNode* curr, wxGraphicsContext* gc, wxPen pen, int level = 0);

};
#endif
