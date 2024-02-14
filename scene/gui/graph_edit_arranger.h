/**************************************************************************/
/*  graph_edit_arranger.h                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/


#ifndef GRAPH_EDIT_ARRANGER_H
#define GRAPH_EDIT_ARRANGER_H

#include "core/object/ref_counted.h"
#include "core/templates/hash_map.h"
#include "core/templates/hash_set.h"
#include "graph_node.h"
#include "graph_edit.h"

class GraphEditArranger : public RefCounted {
private:
	enum SlotType
	{
		IN_PORT,
		OUT_PORT
	};
	GraphEdit *graph_edit = nullptr; 

	const String FROM_NODE = "from_node";
	const String FROM_PORT = "from_port";
	const String TO_NODE = "to_node";
	const String TO_PORT = "to_port";
	
	bool arranging_graph = false;


	Vector2i cell_padding = Vector2i(1,1);
	Vector2i cell_size = Vector2i(100,25);
	HashSet<GraphNode*> arranged_nodes;
	HashSet<Vector2i> covered_cells;
	Vector<Rect2i> chunk_rects;
public:
	void arrange_nodes();
	GraphEditArranger(GraphEdit *p_graph_edit);
private:
	Vector2i get_default_node_position();
	bool arrange_chunk(GraphNode *p_start_node, Vector2i p_grid_position, Rect2i &p_chunk_rect);
	bool arrange_node(GraphNode *p_node, Vector2i p_grid_position, Rect2i &p_chunk_rect);
	void arrange_connected_nodes(Vector<GraphNode*> p_connected_nodes, Vector2i p_suggested_position, bool p_use_extra_offsets, Rect2i &p_total_area);
	Vector<GraphNode*> get_connected_nodes(GraphNode* p_node, SlotType p_slot_type);
	void bubble_sort(Vector<GraphNode*> &nodes, Vector<int> &indices);
	GraphNode* get_leftmost_connected_node(GraphNode *p_node);
	GraphNode* get_leftmost_connected_node_rec(GraphNode* &p_node, int &p_left_distance, HashSet<GraphNode*> &checked_nodes);
	void compare_leftmost_node(Vector<GraphNode*> &p_graph_nodes, GraphNode* &p_furthest, GraphNode* &p_graph_node, HashSet<GraphNode*> &p_checked_nodes, int p_distance_modifier, int furthest_distance);
	Vector2i get_nodes_grid_size(Vector<GraphNode*> &p_nodes);
	Vector2i get_node_grid_size(GraphNode* p_node);
	void add_range_to_covered_cells(GraphNode *p_node, Vector2i p_grid_position);
	void union_covered_cells(GraphNode *p_node, Vector2i p_grid_position, bool &p_covers_existing_nodes);
	List<Ref<GraphEdit::Connection>> get_connections_to_node(StringName name, SlotType port);

}

#endif // GRAPH_EDIT_ARRANGER_H
