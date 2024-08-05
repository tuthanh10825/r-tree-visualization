#ifndef RTREE_H
#define RTREE_H
#include <wx/geometry.h>
#include <vector>
#include <limits.h>
#include <stack>
using std::vector;
typedef vector<wxPoint2DDouble> rectangle;

class rtree
{
public:
	int M;
	int height = 0;
	struct TreeNode
	{
		TreeNode* parent = 0; 
		std::vector<std::vector<wxPoint2DDouble>> children_obj;
		std::vector<std::vector<wxPoint2DDouble>> children_mbrs;
		std::vector<TreeNode*>children;

		//if the node is leaf, we come with the pair (children_mbrs, children_obj). If not, we come with the pair(children_mbrs, children). 
		bool is_leaf = false;
	} *root = 0;
	rtree(int _M) : M(_M) {}
	virtual ~rtree() { clear(); }
	void clear()
	{
		clear(this->root);
	}
	std::vector<std::vector<wxPoint2DDouble>> search(std::vector<wxPoint2DDouble> bounding_rec) //find oid that intersect the bounding rectangle. 
	{
		if (!root) return {};
		std::vector<std::vector<wxPoint2DDouble>> ans;
		search(root, bounding_rec, ans);
		return ans;
	}
	void insert(std::vector<wxPoint2DDouble> object)
	{
		std::vector<wxPoint2DDouble> new_mbr = find_mbr(object);
		if (!root)
		{

			root = new TreeNode(); height = 1; 
			root->children_obj.push_back(object);
			root->children_mbrs.push_back(new_mbr);
			root->children.push_back(0);
			root->is_leaf = true;
			return;
		}

		std::stack<TreeNode*> search_path; search_path.push(0);
		TreeNode* curr = root;
		while (curr)
		{
			const vector<vector<wxPoint2DDouble>>& mbrs = curr->children_mbrs;
			int index = -1; double extended_requirement = DBL_MAX;
			for (int i = 0; i < mbrs.size(); ++i)
			{
				if (area_extending_cost(mbrs[i], new_mbr) < extended_requirement)
				{
					index = i;
					extended_requirement = area_extending_cost(mbrs[i], new_mbr);
				}
				else if (area_extending_cost(mbrs[i], new_mbr) == extended_requirement)
				{
					if (area(mbrs[i]) < area(mbrs[index])) index = i;
				}
			}
			search_path.push(curr);
			curr = curr->children[index];
		}

		search_path.top()->children.push_back(0);
		search_path.top()->children_obj.push_back(object);
		search_path.top()->children_mbrs.push_back(new_mbr);
		while (search_path.size() > 1)
		{

			TreeNode* top = search_path.top(); search_path.pop();
			TreeNode* parent_top = search_path.empty() ? 0 : search_path.top();
			if (top->children_obj.size() == M + 1)
			{
				split(top, parent_top); // split top, with parent_top to easier handling. 
			}
			else
			{
				fix_mbr(parent_top, top); //fix the mbr of parent_top, with top is the mbr which has been changed when inserting new node.
			}
		}
	}
	void remove(const vector<wxPoint2DDouble>& remove_obj)
	{
		rectangle remove_mbr = find_mbr(remove_obj); 
		TreeNode* leaf = find_leaf(root, remove_mbr, remove_obj); 
		vector<rectangle>& mbrs = leaf->children_mbrs; 
		vector<vector<wxPoint2DDouble>>& objs = leaf->children_obj; 
	
		int index = 0; 
		for (; index < mbrs.size(); ++index)
		{
			if (mbrs[index] == remove_mbr && remove_obj == objs[index]) break; 
		}
		assert(index != mbrs.size()); 
		mbrs.erase(mbrs.begin() + index); 
		objs.erase(objs.begin() + index); 
		leaf->children.pop_back(); 
		condense_tree(leaf);
		if (root->children.size() == 1)
		{
			--height; 
			TreeNode* temp = root; 
			root = root->children[0]; 
			if (root) root->parent = 0; 
			delete temp; 
			return; 
		}
		return; 
	}
private:
	void condense_tree(TreeNode *& leaf)
	{
		TreeNode* curr = leaf; 
		vector<TreeNode*> false_delete; 
		while (curr != root)
		{
			TreeNode* parent = curr->parent; 
			int index = 0; 
			for (; index <= parent->children.size(); ++index)
			{
				if (parent->children[index] == curr) break; 
			}
			assert(index != parent->children.size()); 
			if (curr->children.size() < M / 2)
			{
				parent->children.erase(parent -> children.begin() + index); 
				parent->children_mbrs.erase(parent->children_mbrs.begin() + index); 
				parent->children_obj.pop_back(); 
				false_delete.push_back(curr); 
			}
			else
			{
				fix_mbr(parent, curr); 
			}
			curr = parent; 
		}
		//reinsert all the entries of nodes that are in the set false_delete.
		
		int s = 0; 
		for (TreeNode* reinsert : false_delete)
		{
			if (reinsert->is_leaf)
			{
				for (int i = 0; i < reinsert->children_obj.size(); ++i) insert(reinsert -> children_obj[i]);
			}
			else
			{
				for (int i = 0; i < reinsert->children_mbrs.size(); ++i)
				{
					insert_at_height(reinsert->children[i], this -> height - s, reinsert->children_mbrs[i]); 
				}
				
			}
			++s; 
			delete reinsert; 
		}
		
	}
	void insert_at_height(TreeNode* reinsert, int h, rectangle curr_mbr)
	{

		TreeNode* curr = root; 
		std::stack<TreeNode*> search_path; 
		search_path.push(0); 
		for (int i = 1; i < h; ++i)
		{
			const vector<vector<wxPoint2DDouble>>& mbrs = curr->children_mbrs;
			int index = -1; double extended_requirement = DBL_MAX;
			for (int i = 0; i < mbrs.size(); ++i)
			{
				if (area_extending_cost(mbrs[i], curr_mbr) < extended_requirement)
				{
					index = i;
					extended_requirement = area_extending_cost(mbrs[i], curr_mbr);
				}
				else if (area_extending_cost(mbrs[i], curr_mbr) == extended_requirement)
				{
					if (area(mbrs[i]) < area(mbrs[index])) index = i;
				}
			}
			search_path.push(curr);
			curr = curr->children[index];
		}
		search_path.top()->children.push_back(reinsert);
		search_path.top()->children_obj.push_back({});
		search_path.top()->children_mbrs.push_back(curr_mbr);
		while (search_path.size() > 1)
		{

			TreeNode* top = search_path.top(); search_path.pop();
			TreeNode* parent_top = search_path.empty() ? 0 : search_path.top();
			if (top->children_obj.size() == M + 1)
			{
				split(top, parent_top); // split top, with parent_top to easier handling. 
			}
			else
			{
				fix_mbr(parent_top, top); //fix the mbr of parent_top, with top is the mbr which has been changed when inserting new node.
			}
		}
	}
	TreeNode* find_leaf(TreeNode* root, const rectangle & obj_rect, const std::vector<wxPoint2DDouble>& obj)
	{
		if (!root) return 0; 
		const std::vector<std::vector<wxPoint2DDouble>>& candidate = root->children_mbrs;
		const std::vector<TreeNode*>& children = root->children;
		const std::vector<std::vector<wxPoint2DDouble>>& object = root->children_obj;
		if (!(root->is_leaf))
		{
			for (int i = 0; i < candidate.size(); ++i)
			{
				if (is_inside(obj_rect, candidate[i]))
				{
					TreeNode* ans = find_leaf(children[i], obj_rect, obj); 
					if (ans != 0) return ans;
				}
			}
		}
		else
		{
			for (int i = 0; i < candidate.size(); ++i)
			{
				if (candidate[i] == obj_rect && obj == object[i])
					return root; 
			}
		}
		return 0; 
	}
	
	void clear(TreeNode* &curr)
	{
		if (!curr) return; 
		for (TreeNode* child : curr->children)
			clear(child); 
		delete curr; curr = 0; 
		return; 
		
	}
	inline bool is_inside(const rectangle& smaller, const rectangle& bigger)
	{
		return (smaller[0].m_x >= bigger[0].m_x) && (smaller[0].m_y >= bigger[0].m_y) 
			&& (smaller[2].m_x <= bigger[2].m_x) && (smaller[2].m_y <= bigger[2].m_y);
	}
	bool is_overlap(const std::vector<wxPoint2DDouble>& rect1, const std::vector<wxPoint2DDouble>& rect2)
	{
		wxPoint2DDouble tl1 = rect1[0]; wxPoint2DDouble dr1 = rect1[2];
		wxPoint2DDouble tl2 = rect2[0]; wxPoint2DDouble dr2 = rect2[2];
		return std::max(tl1.m_x, tl2.m_x) < std::min(dr1.m_x, dr2.m_x)
			&& std::max(tl1.m_y, tl2.m_y) < std::min(dr2.m_y, dr2.m_y);
	}
	bool is_intersect(const std::vector<wxPoint2DDouble>& rect, const std::vector < wxPoint2DDouble>& obj)
	{
		for (const auto& point : obj)
		{
			if (point.m_x >= rect[0].m_x && point.m_x <= rect[2].m_x && point.m_y <= rect[2].m_y && point.m_y >= rect[0].m_y)
				return true;
		}
		return false;
	}
	void search(TreeNode* root, std::vector<wxPoint2DDouble> bounding_rect, std::vector<std::vector<wxPoint2DDouble>>& ans)
	{
		const std::vector<std::vector<wxPoint2DDouble>>& candidate = root->children_mbrs;
		const std::vector<TreeNode*>& children = root->children;
		const std::vector<std::vector<wxPoint2DDouble>>& object = root->children_obj;
		if (!(root->is_leaf))

		{
			for (int i = 0; i < candidate.size(); ++i)
			{
				if (is_overlap(candidate[i], bounding_rect))
				{
					search(children[i], bounding_rect, ans);
				}
			}
		}
		else
		{
			for (int i = 0; i < candidate.size(); ++i)
			{
				if (is_overlap(candidate[i], bounding_rect) && is_intersect(bounding_rect, object[i]))
					ans.push_back(object[i]);
			}
		}
	}
	void split(TreeNode* curr, TreeNode* parent)
	{
		if (parent == 0)
		{
			++height; 
			parent = new TreeNode();
			parent->children.push_back(curr);
			parent->children_obj.push_back({});
			parent->children_mbrs.push_back({});
			fix_mbr(parent, curr);
		}
		vector<TreeNode*> children = parent->children;
		int index = 0;
		for (; index < children.size(); ++index)
		{
			if (children[index] == curr)
			{
				break;
			}
		}
		children = curr->children;
		vector<vector<wxPoint2DDouble>>& children_mbrs = curr->children_mbrs;
		vector<vector<wxPoint2DDouble>>& children_obj = curr->children_obj;
		vector<wxPoint2DDouble> e1 = children_mbrs[0];
		vector<wxPoint2DDouble> e2 = children_mbrs[1];

		for (int i = 0; i < children_mbrs.size(); ++i)
		{
			for (int j = i + 1; j < children_mbrs.size(); ++j)
			{
				if (dead_space_area(children_mbrs[j], children_mbrs[i]) > dead_space_area(e1, e2))
				{
					e1 = children_mbrs[i];
					e2 = children_mbrs[j];
				}
			}
		}
		rectangle new_mbr1 = e1;
		rectangle new_mbr2 = e2;
		TreeNode* new_node1 = new TreeNode();
		TreeNode* new_node2 = new TreeNode();
		int entries_left = M - 1;
 		for (int i = 0; i < children_mbrs.size(); ++i)
		{
			if (children_mbrs[i] == e1)
			{
				new_node1->children_mbrs.push_back(children_mbrs[i]);
				new_node1->children_obj.push_back(children_obj[i]);
				new_node1->children.push_back(children[i]);
				continue; 
			}
			else if (children_mbrs[i] == e2)
			{
				new_node2->children.push_back(children[i]);
				new_node2->children_obj.push_back(children_obj[i]);
				new_node2->children_mbrs.push_back(children_mbrs[i]);
				continue; 
			}
			--entries_left;
			if (area_extending_cost(new_mbr1, children_mbrs[i]) < area_extending_cost(new_mbr2, children_mbrs[i])
				|| new_node1->children.size() == M - entries_left)
			{
				new_node1->children_mbrs.push_back(children_mbrs[i]);
				new_node1->children_obj.push_back(children_obj[i]);
				new_node1->children.push_back(children[i]);

				new_mbr1[4].m_x = new_mbr1[0].m_x = new_mbr1[3].m_x = std::min(children_mbrs[i][0].m_x, new_mbr1[0].m_x);
				new_mbr1[4].m_y = new_mbr1[0].m_y = new_mbr1[1].m_y = std::min(children_mbrs[i][0].m_y, new_mbr1[0].m_y);

				new_mbr1[1].m_x = new_mbr1[2].m_x = std::max(children_mbrs[i][2].m_x, new_mbr1[2].m_x);
				new_mbr1[3].m_y = new_mbr1[2].m_y = std::max(children_mbrs[i][2].m_y, new_mbr1[2].m_y);

			}
			else if (area_extending_cost(new_mbr1, children_mbrs[i]) > area_extending_cost(new_mbr2, children_mbrs[i])
				|| new_node2->children.size() == M - entries_left)
			{
				new_node2->children.push_back(children[i]);
				new_node2->children_obj.push_back(children_obj[i]);
				new_node2->children_mbrs.push_back(children_mbrs[i]);

				new_mbr2[4].m_x = new_mbr2[0].m_x = new_mbr2[3].m_x = std::min(children_mbrs[i][0].m_x, new_mbr2[0].m_x);
				new_mbr2[4].m_y = new_mbr2[0].m_y = new_mbr2[1].m_y = std::min(children_mbrs[i][0].m_y, new_mbr2[0].m_y);

				new_mbr2[1].m_x = new_mbr2[2].m_x = std::max(children_mbrs[i][2].m_x, new_mbr2[2].m_x);
				new_mbr2[3].m_y = new_mbr2[2].m_y = std::max(children_mbrs[i][2].m_y, new_mbr2[2].m_y);
			}
			else
			{
				if (area(new_mbr1) < area(new_mbr2))
				{
					new_node1->children_mbrs.push_back(children_mbrs[i]);
					new_node1->children_obj.push_back(children_obj[i]);
					new_node1->children.push_back(children[i]);

					new_mbr1[4].m_x = new_mbr1[0].m_x = new_mbr1[3].m_x = std::min(children_mbrs[i][0].m_x, new_mbr1[0].m_x);
					new_mbr1[4].m_y = new_mbr1[0].m_y = new_mbr1[1].m_y = std::min(children_mbrs[i][0].m_y, new_mbr1[0].m_y);

					new_mbr1[1].m_x = new_mbr1[2].m_x = std::max(children_mbrs[i][2].m_x, new_mbr1[2].m_x);
					new_mbr1[3].m_y = new_mbr1[2].m_y = std::max(children_mbrs[i][2].m_y, new_mbr1[2].m_y);
				}
				else
				{
					new_node2->children.push_back(children[i]);
					new_node2->children_obj.push_back(children_obj[i]);
					new_node2->children_mbrs.push_back(children_mbrs[i]);

					new_mbr2[4].m_x = new_mbr2[0].m_x = new_mbr2[3].m_x = std::min(children_mbrs[i][0].m_x, new_mbr2[0].m_x);
					new_mbr2[4].m_y = new_mbr2[0].m_y = new_mbr2[1].m_y = std::min(children_mbrs[i][0].m_y, new_mbr2[0].m_y);

					new_mbr2[1].m_x = new_mbr2[2].m_x = std::max(children_mbrs[i][2].m_x, new_mbr2[2].m_x);
					new_mbr2[3].m_y = new_mbr2[2].m_y = std::max(children_mbrs[i][2].m_y, new_mbr2[2].m_y);
				}
			}
		}
		new_node1->is_leaf = new_node2->is_leaf = curr->is_leaf;
		new_node1->parent = new_node2->parent = parent; 

		parent->children[index] = new_node1;
		parent->children_mbrs[index] = new_mbr1;

		parent->children.push_back(new_node2);
		parent->children_mbrs.push_back(new_mbr2);
		parent->children_obj.push_back({});

		if (curr == root)
		{
			root = parent;
		}
		delete curr;
		return;
	}
	void fix_mbr(TreeNode* parent, TreeNode* curr)// fix the mbr of the curr.
	{
		if (!parent) return; 
		int index = 0; 
		for (; index < parent->children.size(); ++index)
		{
			if (parent->children[index] == curr) break; 
		}
		rectangle resulting_mbr = curr->children_mbrs[0]; 
		for (const rectangle& entries : curr->children_mbrs)
		{
			resulting_mbr[4].m_x = resulting_mbr[1].m_x = resulting_mbr[0].m_x = std::min(resulting_mbr[0].m_x, entries[0].m_x); 
			resulting_mbr[4].m_y = resulting_mbr[3].m_y = resulting_mbr[0].m_y = std::min(resulting_mbr[0].m_y, entries[0].m_y); 

			resulting_mbr[2].m_x = resulting_mbr[3].m_x = std::max(resulting_mbr[2].m_x, entries[2].m_x); 
			resulting_mbr[2].m_y = resulting_mbr[1].m_y = std::max(resulting_mbr[2].m_y, entries[2].m_y); 
 		}
		parent->children_mbrs[index] = resulting_mbr; 
		return; 
	}
	std::vector<wxPoint2DDouble> find_mbr(std::vector<wxPoint2DDouble> obj)
	{
		if (obj.empty()) return {};
		wxDouble leftmost = DBL_MAX, rightmost = DBL_MIN, topmost = DBL_MAX, bottommost = DBL_MIN;
		for (const auto& element : obj)
		{
			leftmost = std::min(leftmost, element.m_x);
			rightmost = std::max(rightmost, element.m_x);
			topmost = std::min(topmost, element.m_y);
			bottommost = std::max(bottommost, element.m_y);
		}

		return {
			wxPoint2DDouble(leftmost, topmost),
			wxPoint2DDouble(rightmost, topmost),
			wxPoint2DDouble(rightmost, bottommost),
			wxPoint2DDouble(leftmost, bottommost),
			wxPoint2DDouble(leftmost, topmost)
		};

	}
	double area_extending_cost(const vector<wxPoint2DDouble>& added, const vector<wxPoint2DDouble>& adding)
	{
		double left = std::min({ added[0].m_x, added[2].m_x, adding[0].m_x, adding[2].m_x });
		double right = std::max({ added[0].m_x, added[2].m_x, adding[0].m_x, adding[2].m_x });
		double up = std::min({ added[0].m_y, added[2].m_y, adding[0].m_y, adding[2].m_y });
		double down = std::max({ added[0].m_y, added[2].m_y, adding[0].m_y, adding[2].m_y });

		return (right - left) * (down - up) - std::abs(added[0].m_x - added[2].m_x) * std::abs(added[0].m_y - added[2].m_y);
	}
	inline double area(const vector<wxPoint2DDouble>& rect) { return std::abs(rect[0].m_x - rect[2].m_x) * std::abs(rect[0].m_y - rect[2].m_y); }
	double overlap_area(const vector<wxPoint2DDouble>& rect1, const vector<wxPoint2DDouble>& rect2)
	{
		return std::max(std::max(rect1[0].m_x, rect2[0].m_x) - std::min(rect1[2].m_x, rect2[2].m_x), 0.0) *
			std::max(std::max(rect1[0].m_y, rect2[0].m_y) - std::min(rect1[2].m_y, rect2[2].m_y), 0.0);
	}
	double dead_space_area(rectangle& r1, rectangle& r2)
	{
		double left = std::min({ r1[0].m_x, r1[2].m_x, r2[0].m_x, r2[2].m_x });
		double right = std::max({ r1[0].m_x, r1[2].m_x, r2[0].m_x, r2[2].m_x });
		double up = std::min({ r1[0].m_y, r1[2].m_y, r2[0].m_y, r2[2].m_y });
		double down = std::max({ r1[0].m_y, r1[2].m_y, r2[0].m_y, r2[2].m_y });
		return std::abs(right - left) * std::abs(down - up) - (area(r1) + area(r2) - overlap_area(r1, r2));
	}
};

#endif