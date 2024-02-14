/**************************************************************************/
/*  graph_edit_arranger.cpp                                               */
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

#include "graph_edit_arranger.h"

GraphEditArranger::GraphEditArranger(GraphEdit *p_edit) {
    graph_edit = p_edit;
}

Vector2i GraphEditArranger::get_default_node_position() {
	return (Vector2i)((graph_edit->get_scroll_offset() + graph_edit->get_size() / 6) / graph_edit->get_zoom() / cell_size);
}

bool GraphEditArranger::arrange_chunk(GraphNode *p_start_node, Vector2i p_grid_position, Rect2i &p_chunk_rect) {
	const Rect2i base_value = Rect2i(0,0,0,0);
	bool is_default = false;
	while (!is_default) {
		for (Rect2i rect : chunk_rects) {
			if (rect.has_point(p_grid_position * cell_size) && rect == base_value) {
				is_default = true;
			} else {
				p_grid_position += Vector2i(0, 1);
			}
		}
	}
	return arrange_node(p_start_node, p_grid_position, p_chunk_rect);
}

bool GraphEditArranger::arrange_node(GraphNode *p_node, Vector2i p_grid_position, Rect2i &p_chunk_rect) {
	if (!p_node->is_selected()) {
		return false;
	}
	arranged_nodes.insert(p_node);

	Vector<GraphNode *> input_nodes = get_connected_nodes(p_node, SlotType::IN_PORT);
	Vector<GraphNode *> output_nodes = get_connected_nodes(p_node, SlotType::OUT_PORT);

	Vector2i final_grid_position;

	int x = 0;
	while (true) {
		final_grid_position = Vector2i(p_grid_position.x, p_grid_position.y + x);
		bool covers_existing_nodes = false;
		union_covered_cells(p_node, final_grid_position, covers_existing_nodes);
		if (covers_existing_nodes) {
			++x;
			continue;
		}
		add_range_to_covered_cells(p_node, final_grid_position);
		p_node->set_position_offset(final_grid_position * cell_size);
		break;
	}

	Rect2i total_area = Rect2(p_node->get_position_offset(), p_node->get_size());

	arrange_connected_nodes(output_nodes, final_grid_position + Vector2i(get_node_grid_size(p_node).x, -get_nodes_grid_size(output_nodes).y / 2), false, total_area);
	arrange_connected_nodes(input_nodes, final_grid_position + Vector2i(0, get_nodes_grid_size(input_nodes).y / 2), true, total_area);

	p_chunk_rect = total_area;

	return true;
}

void GraphEditArranger::arrange_connected_nodes(Vector<GraphNode *> p_connected_nodes, Vector2i p_suggested_position, bool p_use_extra_offsets, Rect2i &p_total_area) {
	for (int x = 0; x < p_connected_nodes.size(); x++) {
		GraphNode *graph_node = p_connected_nodes[x];

		if (!arranged_nodes.has(graph_node)) {
			Vector2i local_suggested_position = p_suggested_position;
			if (p_use_extra_offsets) {
				local_suggested_position -= get_node_grid_size(graph_node);
			}
			Rect2i arranged_node_area;
			bool could_arrange = arrange_node(graph_node, local_suggested_position, arranged_node_area);

			if (could_arrange) {
				p_total_area = p_total_area.merge(arranged_node_area);
			}
		}
	}
}

Vector<GraphNode *> GraphEditArranger::get_connected_nodes(GraphNode *p_node, SlotType p_slot_type) {
	List<Ref<GraphEdit::Connection>> graph_connections = get_connections_to_node(p_node->get_name(), p_slot_type);

	Vector<GraphNode *> connected_nodes_list;
	Vector<int> connected_nodes_indexes_list;

	for (Ref<GraphEdit::Connection> con : graph_connections) {
		GraphNode *connected_node;
		StringName path;
		switch (p_slot_type) {
			case IN_PORT:
				path = (*con)->from_node;
				connected_node = dynamic_cast<GraphNode*>(graph_edit->get_node(NodePath(path)));
				break;
			case OUT_PORT:
				path = (*con)->to_node;
				connected_node = dynamic_cast<GraphNode*>(graph_edit->get_node(NodePath(path)));
				break;
		}

		if (connected_node != p_node) {
			int existing_index = connected_nodes_list.find(connected_node);
			int other_index;
			if (p_slot_type == IN_PORT) {
				other_index = con->to_port;
			}
			else {
				other_index = con->from_port;
			}

			if (existing_index == -1) {
				connected_nodes_list.push_back(connected_node);
				connected_nodes_indexes_list.push_back(other_index);
			} else {
				int new_index = (connected_nodes_indexes_list[existing_index] + other_index) / 2;
				connected_nodes_indexes_list.set(existing_index, new_index);
			}
		}

		bubble_sort(connected_nodes_list, connected_nodes_indexes_list);
		return connected_nodes_list;
	}
}

void GraphEditArranger::bubble_sort(Vector<GraphNode *> &nodes, Vector<int> &indices) {
	int n = nodes.size();
	bool swapped;
	do {
		swapped = false;
		for (int i = 1; i < n; ++i) {
			if (nodes.get(i - 1) > nodes[i]) {
				// Swap nodes
				GraphNode *tempKey = nodes[i - 1];
				nodes.set(i - 1, nodes[i]);
				nodes.set(i, tempKey);

				// Swap indices
				int tempValue = indices[i - 1];
				indices.set(i - 1, indices[i]);
				indices.set(i,tempValue);
			}
		}
		// At the end of each pass, the largest element will be at the end,
		// so we can reduce the range to search in by 1.
		--n;
	} while (swapped);
}

GraphNode *GraphEditArranger::get_leftmost_connected_node(GraphNode *p_node) {
	HashSet<GraphNode *> checked_nodes;
	int left_distance;
	GraphNode *graph_node = get_leftmost_connected_node_rec(p_node, left_distance, checked_nodes);
	return graph_node;
}

GraphNode *GraphEditArranger::get_leftmost_connected_node_rec(GraphNode* &p_node, int &p_left_distance, HashSet<GraphNode *> &checked_nodes) {
	if (!p_node->is_selected()) {
		p_left_distance = 0;
		return nullptr;
	}

	checked_nodes.insert(p_node);

	Vector<GraphNode *> input_nodes = get_connected_nodes(p_node, SlotType::IN_PORT);
	Vector<GraphNode *> output_nodes = get_connected_nodes(p_node, SlotType::IN_PORT);

	GraphNode *furthest = nullptr;
	int furthest_distance = 0;

	compare_leftmost_node(output_nodes, furthest, p_node, checked_nodes,  -1, furthest_distance);
	compare_leftmost_node(input_nodes, furthest, p_node, checked_nodes, 1, furthest_distance);

	if (!furthest) {
		p_left_distance = 0;
		return p_node;
	}

	p_left_distance = furthest_distance;
	return furthest;
}

void GraphEditArranger::compare_leftmost_node(Vector<GraphNode *> &p_graph_nodes, GraphNode *&p_furthest, GraphNode *&p_graph_node, HashSet<GraphNode *> &p_checked_nodes, int p_distance_modifier, int furthest_distance) {
	for (GraphNode *graph_node : p_graph_nodes) {
		int distance;
		GraphNode *leftmost_node = get_leftmost_connected_node_rec(p_graph_node, distance, p_checked_nodes);
		if (leftmost_node && distance + p_distance_modifier > furthest_distance) {
			p_furthest = leftmost_node;
			furthest_distance = distance + p_distance_modifier;
		}
	}
}

Vector2i GraphEditArranger::get_nodes_grid_size(Vector<GraphNode *> &p_nodes) {
	Vector2i grid_size = Vector2i(0, 0);
	for (GraphNode *node : p_nodes) {
		grid_size += get_node_grid_size(node);
	}
	return grid_size;
}

Vector2i GraphEditArranger::get_node_grid_size(GraphNode *p_node) {
	return static_cast<Vector2i>((p_node->get_size() / cell_size).ceil()) + cell_padding * 2;
}

void GraphEditArranger::add_range_to_covered_cells(GraphNode *p_node, Vector2i p_grid_position) {
	p_grid_position -= cell_padding;
	Vector2i covered_area = get_node_grid_size(p_node);

	for (int x = 0; x < covered_area.x; x++) {
		for (int y = 0; y < covered_area.y; y++) {
			Vector2i cell_location = p_grid_position + Vector2i(x, y);
			covered_cells.insert(cell_location);
		}
	}
}

void GraphEditArranger::union_covered_cells(GraphNode *p_node, Vector2i p_grid_position, bool &p_covers_existing_nodes) {
	p_grid_position -= cell_padding;
	Vector2i covered_area = get_node_grid_size(p_node);

	for (int x = 0; x < covered_area.x; x++) {
		for (int y = 0; y < covered_area.y; y++) {
			Vector2i cell_location = p_grid_position + Vector2i(x, y);
			p_covers_existing_nodes = p_covers_existing_nodes || covered_cells.has(cell_location);
		}
	}
}

List<Ref<GraphEdit::Connection>> GraphEditArranger::get_connections_to_node(StringName name, SlotType port) {
	List<Ref<GraphEdit::Connection>> graph_connections;

	for (Ref<GraphEdit::Connection> l : graph_edit->get_connection_list()) {
		switch (port) {
			case IN_PORT:
				if ((*l)->from_node == name) {
					graph_connections.push_back(l);
				}
				break;
			case OUT_PORT:
				if ((*l)->to_node == name) {
					graph_connections.push_back(l);
				}
				break;
		}
	}

	return graph_connections;
}

void GraphEditArranger::arrange_nodes() {
	ERR_FAIL_NULL(graph_edit);

	if (!arranging_graph) {
		arranging_graph = true;
	} else {
		return;
	}

	Vector<GraphNode *> graph_nodes;

	for (int i = graph_edit->get_child_count() - 1; i >= 0; i--) {
		GraphNode *graph_node = Object::cast_to<GraphNode>(graph_edit->get_child(i));
		if (graph_node != nullptr && graph_node->is_selected()) {
			graph_nodes.push_back(graph_node);
		}
	}

	for (GraphNode *graph_node : graph_nodes) {
		if (!arranged_nodes.has(graph_node)) {
			GraphNode *leftmost_node = get_leftmost_connected_node(graph_node);
			Vector2i position = static_cast<Vector2i>(leftmost_node->get_position_offset() / cell_size);

			Rect2i chunk_rect;
			bool could_get_chunk_rect = arrange_chunk(leftmost_node, position, chunk_rect);
			if (!could_get_chunk_rect) {
				chunk_rects.push_back(chunk_rect);
			}
		}
	}

	arranged_nodes.clear();
	covered_cells.clear();
	chunk_rects.clear();
}
