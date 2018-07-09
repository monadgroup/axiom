use mir;
use std::collections::VecDeque;

type NodeRef = usize;
type ValueSocketRef = (NodeRef, usize);

pub fn order_nodes(surface: &mut mir::Surface) {
    let outputs = get_output_nodes(&surface);
    let associated_sockets = get_associated_sockets(&surface);
    let ordered_nodes = get_ordered_nodes(&surface.nodes, &outputs, &associated_sockets);
    surface.nodes = ordered_nodes;
}

fn get_ordered_nodes(
    nodes: &[mir::Node],
    outputs: &[NodeRef],
    value_groups: &[Vec<ValueSocketRef>],
) -> Vec<mir::Node> {
    let mut inverse_order = Vec::new();
    let mut is_node_visited = vec![false; nodes.len()];
    let mut node_queue = VecDeque::new();

    for &output_node in outputs {
        is_node_visited[output_node] = true;
        node_queue.push_back(output_node);
    }

    while let Some(next_node) = node_queue.pop_front() {
        inverse_order.push(nodes[next_node].clone());

        for socket in &nodes[next_node].sockets {
            // walk to other nodes through sockets _if the socket writes to the group_.
            for &(connected_node_ref, connected_socket_ref) in &value_groups[socket.group_id] {
                let connected_socket = &nodes[connected_node_ref].sockets[connected_socket_ref];
                if !connected_socket.value_written || is_node_visited[connected_node_ref] {
                    continue;
                }

                is_node_visited[connected_node_ref] = true;
                node_queue.push_back(connected_node_ref);
            }
        }
    }

    // For UX, add nodes that don't affect the output (i.e aren't connected in the flow) so the user
    // can still play with them. We can't really determine the order of these, since there's no
    // output.
    // Todo: only put these in when in the editor
    if inverse_order.len() < nodes.len() {
        for (node_index, &was_visited) in is_node_visited.iter().enumerate() {
            if !was_visited {
                inverse_order.push(nodes[node_index].clone());
            }
        }
    }

    inverse_order.reverse();
    inverse_order
}

/// Finds sockets that are connected to each value group
fn get_associated_sockets(surface: &mir::Surface) -> Vec<Vec<ValueSocketRef>> {
    let mut group_sockets = vec![Vec::new(); surface.groups.len()];

    for (node_index, node) in surface.nodes.iter().enumerate() {
        for (socket_index, socket) in node.sockets.iter().enumerate() {
            group_sockets[socket.group_id].push((node_index, socket_index));
        }
    }

    group_sockets
}

/// Finds output nodes in the provided surface
/// An output node is a node with a socket that is:
///   - Connected to a value group with a socket source, AND
///   - Is written to by the node.
///
/// An output socket is a socket connected to a value group that gets its value from an external
/// socket, AND is written to by the node.
fn get_output_nodes(surface: &mir::Surface) -> Vec<NodeRef> {
    let mut result = Vec::new();

    for (node_index, node) in surface.nodes.iter().enumerate() {
        for socket in node.sockets.iter() {
            if socket.value_written {
                if let mir::ValueGroupSource::Socket(_) = surface.groups[socket.group_id].source {
                    result.push(node_index);
                    break;
                }
            }
        }
    }

    result
}
