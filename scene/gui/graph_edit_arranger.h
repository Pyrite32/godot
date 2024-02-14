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
#include "scene/gui/graph_node.h"

class GraphEdit;

class GraphEditArranger : public RefCounted {
private:
	enum SlotType
	{
		IN_PORT,
		OUT_PORT
	};

	const String FROM_NODE = "from_node";
	const String FROM_PORT = "from_port";
	const String TO_NODE = "to_node";
	const String TO_PORT = "to_port";
	
	bool arranging_graph = false;

	Vector2i cell_padding = Vector2i(1,1);
	Vector2i cell_size = Vector2i(100,25);
	GraphEdit *graph_edit = nullptr;
	HashSet<GraphNode*> arranged_nodes;
	HashSet<Vector2i> covered_cells;
	Vector<Rect2i> chunk_rects;
public:
	void arrange_nodes();
private:
	Vector2i get_default_node_position();
	bool arrange_chunk(GraphNode *p_start_node, Vector2i p_grid_position, Rect2i &p_chunk_rect);
	bool arrange_node(GraphNode *p_node, Vector2i p_grid_position, Rect2i &p_chunk_rect);
	void arrange_connected_nodes(Vector<GraphNode*> p_connected_nodes, Vector2i p_suggested_position, bool p_use_extra_offsets, Rect2i &p_total_area);
	Vector<GraphNode*> get_connected_nodes(GraphNode* p_node, SlotType p_slot_type);
	void bubble_sort(Vector<GraphNode*> &nodes, Vector<int> &indices);
	GraphNode* get_leftmost_connected_node(GraphNode *p_node);
	GraphNode* get_leftmost_connected_node_rec(GraphNode &p_node, int &p_left_distance, HashSet<GraphNode*> &checked_nodes);
	void compare_leftmost_node(Vector<GraphNode*> &p_graph_nodes, GraphNode* &p_furthest, GraphNode* &p_graph_node, HashSet<GraphNode*> &p_checked_nodes, int p_distance_modifier, int furthest_distance);
	Vector2i get_nodes_grid_size(Vector<GraphNode*> &p_nodes);
	Vector2i get_node_grid_size(GraphNode* p_node);
	void add_range_to_covered_cells(GraphNode *p_node, Vector2i p_grid_position);
	void union_covered_cells(GraphNode *p_node, Vector2i p_grid_position, bool &p_covers_existing_nodes);
	Vector<Dictionary> get_connections_to_node(StringName name, SlotType port);

}

/*
class GraphEditArranger : public RefCounted {
	enum SET_OPERATIONS {
		IS_EQUAL,
		IS_SUBSET,
		DIFFERENCE,
		UNION,
	};

	GraphEdit *graph_edit = nullptr;
	bool arranging_graph = false;

	int _set_operations(SET_OPERATIONS p_operation, HashSet<StringName> &r_u, const HashSet<StringName> &r_v);
	HashMap<int, Vector<StringName>> _layering(const HashSet<StringName> &r_selected_nodes, const HashMap<StringName, HashSet<StringName>> &r_upper_neighbours);
	Vector<StringName> _split(const Vector<StringName> &r_layer, const HashMap<StringName, Dictionary> &r_crossings);
	void _horizontal_alignment(Dictionary &r_root, Dictionary &r_align, const HashMap<int, Vector<StringName>> &r_layers, const HashMap<StringName, HashSet<StringName>> &r_upper_neighbours, const HashSet<StringName> &r_selected_nodes);
	void _crossing_minimisation(HashMap<int, Vector<StringName>> &r_layers, const HashMap<StringName, HashSet<StringName>> &r_upper_neighbours);
	void _calculate_inner_shifts(Dictionary &r_inner_shifts, const Dictionary &r_root, const Dictionary &r_node_names, const Dictionary &r_align, const HashSet<StringName> &r_block_heads, const HashMap<StringName, Pair<int, int>> &r_port_info);
	float _calculate_threshold(StringName p_v, StringName p_w, const Dictionary &r_node_names, const HashMap<int, Vector<StringName>> &r_layers, const Dictionary &r_root, const Dictionary &r_align, const Dictionary &r_inner_shift, real_t p_current_threshold, const HashMap<StringName, Vector2> &r_node_positions);
	void _place_block(StringName p_v, float p_delta, const HashMap<int, Vector<StringName>> &r_layers, const Dictionary &r_root, const Dictionary &r_align, const Dictionary &r_node_name, const Dictionary &r_inner_shift, Dictionary &r_sink, Dictionary &r_shift, HashMap<StringName, Vector2> &r_node_positions);

public:
	void arrange_nodes();

	GraphEditArranger(GraphEdit *p_graph_edit) :
			graph_edit(p_graph_edit) {}
};
*/




#endif // GRAPH_EDIT_ARRANGER_H
