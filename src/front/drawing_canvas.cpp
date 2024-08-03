#include "drawing_canvas.h"
#include <algorithm>
drawing_canvas::drawing_canvas(wxWindow* parent, wxWindowID id, wxPoint position, wxSize size) : 
	wxWindow (parent, id, position, size)
{
	this->SetBackgroundColour(*wxWHITE); 
	this->SetBackgroundStyle(wxBG_STYLE_PAINT);
	this->Bind(wxEVT_PAINT, &drawing_canvas::on_paint, this);
	this->Bind(wxEVT_LEFT_DOWN, &drawing_canvas::on_button_down, this); 
	this->Bind(wxEVT_MOTION, &drawing_canvas::on_button_move, this); 
	this->Bind(wxEVT_LEFT_UP, &drawing_canvas::on_button_up, this); 
	this->Bind(wxEVT_LEAVE_WINDOW, &drawing_canvas::on_mouse_leave, this); 
}

void drawing_canvas::on_paint(wxPaintEvent& evt)
{
	wxAutoBufferedPaintDC dc(this); 
	dc.Clear(); 
	
	wxGraphicsContext* gc = wxGraphicsContext::Create(dc); 
	if (gc)
	{
		wxPen tree_pen(*wxBLACK, 2 * tree.height - 1);
		gc->SetPen(tree_pen); 
		draw_tree(this->tree.root, gc, tree_pen);
		gc->SetPen(*wxBLACK); 
		if (!current_object.empty())
			gc->StrokeLines(current_object.size(), current_object.data()); 
		if (!choosing_rect.empty())
			gc->StrokeLines(choosing_rect.size(), choosing_rect.data()); 
	}
	delete gc; 
}
void drawing_canvas::draw_tree(rtree::TreeNode *curr, wxGraphicsContext*gc, wxPen pen, int level)
{
	if (!curr) return; 
	gc->SetPen(pen);
	for (const rectangle& mbr : curr->children_mbrs)
		gc->StrokeLines(mbr.size(), mbr.data());
	if (curr->is_leaf)
	{
		for (const vector<wxPoint2DDouble>& obj : curr->children_obj)
		{
			if (std::find(choosing_obj.begin(), choosing_obj.end(), obj) != choosing_obj.end())
			{
				gc->SetPen(*wxRED); 
				gc->StrokeLines(obj.size(), obj.data());
				gc->SetPen(pen);
			}
			else gc->StrokeLines(obj.size(), obj.data());
		}
	}
	pen.SetWidth(pen.GetWidth() - 2);
	for (rtree::TreeNode* child : curr->children)
		draw_tree(child, gc, pen, level + 1);
	

}

void drawing_canvas::on_mouse_leave(wxMouseEvent&)
{
	is_drawing = false; 
	if (!current_object.empty()) tree.insert(current_object);
	current_object.clear();
	this->Refresh();
}



void drawing_canvas::on_button_up(wxMouseEvent&)
{
	
	is_drawing = false; 
	if (!current_object.empty()) tree.insert(current_object); 
	current_object.clear();
	this->Refresh();
}

void drawing_canvas::on_button_move(wxMouseEvent& event)
{
	if (is_drawing)
	{
		current_object.push_back(event.GetPosition());
		this->Refresh();
	}
}

void drawing_canvas::on_button_down(wxMouseEvent&)
{
	is_drawing = true; 
}


void drawing_canvas::clear()
{
	this->tree.clear(); 
	this->choosing_rect.clear(); 
	this->Refresh();
}


void drawing_canvas::choosing_rect_down(wxMouseEvent& evt)
{
	is_choosing = true;
	choosing_obj.clear(); 
	choosing_rect.clear(); 
	choosing_rect.push_back(wxPoint2DDouble(evt.GetPosition()));
}

void drawing_canvas::choosing_rect_move(wxMouseEvent& evt)
{
	if (is_choosing)
	{
		wxPoint2DDouble curr = wxPoint2DDouble(evt.GetPosition());
		std::vector<wxPoint2DDouble> curr_rect = {
			choosing_rect[0],
			wxPoint2DDouble(choosing_rect[0].m_x, curr.m_y),
			curr,
			wxPoint2DDouble(curr.m_x, choosing_rect[0].m_y),
			choosing_rect[0]
		};
		this->Refresh();
		choosing_rect = curr_rect;
	}
}

void drawing_canvas::choosing_rect_up(wxMouseEvent& evt)
{

	if (is_choosing)
	{
		
		
		
		is_choosing = false;
		if (choosing_rect.size() == 5)
		{
			choosing_rect.pop_back();
			std::sort(choosing_rect.begin(), choosing_rect.end(), [](wxPoint2DDouble& p1, wxPoint2DDouble& p2)
				{
					return (p1.m_y != p2.m_y ?
						p1.m_y < p2.m_y :
						p1.m_x < p2.m_x);

				});
			std::swap(choosing_rect[2], choosing_rect[3]);
			choosing_rect.push_back(choosing_rect[0]);
			for (const auto& obj : tree.search(choosing_rect))
				choosing_obj.push_back(obj);
		}
		this->Refresh();
	}
}

void drawing_canvas::choosing_rect_leave(wxMouseEvent&)
{
	if (is_choosing)

	{
		is_choosing = false;
		if (choosing_rect.size() == 5)
		{
			choosing_rect.pop_back();
			std::sort(choosing_rect.begin(), choosing_rect.end(), [](wxPoint2DDouble& p1, wxPoint2DDouble& p2)
				{
					return (p1.m_y != p2.m_y ?
						p1.m_y < p2.m_y :
						p1.m_x < p2.m_x);

				});
			std::swap(choosing_rect[2], choosing_rect[3]);
			choosing_rect.push_back(choosing_rect[0]);
			for (const auto& obj : tree.search(choosing_rect))
				choosing_obj.push_back(obj);
		}
		this->Refresh();
	}
}