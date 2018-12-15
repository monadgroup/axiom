use crate::mir;
use std::collections::{HashMap, HashSet};

// flatten groups that are only used once
pub fn flatten_groups(surfaces: &mut HashMap<mir::SurfaceRef, mir::Surface>) {
    let flatten_candidates = find_flatten_candidates(surfaces.values());

    println!("Flatten candidates: {:?}", flatten_candidates);

    let mut visited_surfaces = HashSet::new();

    let replaced_root = visit_surface(0, &mut visited_surfaces, &flatten_candidates, surfaces);
    surfaces.insert(0, replaced_root);
}

fn visit_surface(
    surface_id: mir::SurfaceRef,
    visited_surfaces: &mut HashSet<mir::SurfaceRef>,
    flatten_candidates: &HashSet<mir::SurfaceRef>,
    all_surfaces: &mut HashMap<mir::SurfaceRef, mir::Surface>,
) -> mir::Surface {
    // Find all surface this one depends on, and process them.
    // Note: we move the surface out of the map since later on we also need to remove the surface
    // we're pulling into this one, and we need to prove to the borrow-checker that we're not trying
    // to pull something out of the map that we currently have a reference to.  Yay for memory
    // safety!
    let mut surface = all_surfaces.remove(&surface_id).unwrap();

    if !visited_surfaces.insert(surface_id) {
        return surface;
    }

    let mut node_index = 0;
    while node_index < surface.nodes.len() {
        let node = &surface.nodes[node_index];
        let (is_plain_group, subsurface_id) = match node.data {
            mir::NodeData::Group(surface_id) => (true, surface_id),
            mir::NodeData::ExtractGroup { surface, .. } => (false, surface),
            _ => {
                node_index += 1;
                continue;
            }
        };

        // Recurse into the child surface to process it, then try to flatten it.
        let source_surface = visit_surface(
            subsurface_id,
            visited_surfaces,
            flatten_candidates,
            all_surfaces,
        );

        // Track the nodes length so we know how much to increment by to get to the original node.
        // We don't need to check through all of the imported nodes, since we're processing this
        // depth-first so we already know any sub-sub-surfaces have been flattened if possible.
        let start_nodes_len = surface.nodes.len();
        if is_plain_group && flatten_candidates.contains(&subsurface_id) {
            flatten_node_in_surface(node_index, source_surface, &mut surface);
        } else {
            all_surfaces.insert(source_surface.id.id, source_surface);
        }
        let end_nodes_len = surface.nodes.len();

        // note: order of operations is important here, since there is a possibility that the
        // number of nodes in the surface has gone down by one when flattening an empty surface.
        node_index += end_nodes_len + 1 - start_nodes_len;
    }

    surface
}

fn flatten_node_in_surface(
    flatten_place_index: usize,
    source_surface: mir::Surface,
    dest_surface: &mut mir::Surface,
) {
    let replace_node = dest_surface.nodes.remove(flatten_place_index);

    let value_group_map = build_value_group_map(
        &replace_node.sockets,
        &source_surface.groups,
        &mut dest_surface.groups,
    );

    // Insert all nodes in the source surface, while remapping sockets
    dest_surface.nodes.splice(
        flatten_place_index..flatten_place_index,
        source_surface.nodes.into_iter().map(|mut move_node| {
            for node_socket in &mut move_node.sockets {
                node_socket.group_id = value_group_map[node_socket.group_id];
            }
            move_node
        }),
    );
}

fn build_value_group_map(
    group_sockets: &[mir::ValueSocket],
    group_value_groups: &[mir::ValueGroup],
    dest_value_groups: &mut Vec<mir::ValueGroup>,
) -> Vec<usize> {
    group_value_groups
        .iter()
        .map(|value_group| {
            match value_group.source {
                mir::ValueGroupSource::Socket(socket_index) => {
                    // Look at where the socket points to and use that for the mapping
                    group_sockets[socket_index].group_id
                }
                _ => {
                    let new_value_group_index = dest_value_groups.len();
                    dest_value_groups.push(value_group.clone());
                    new_value_group_index
                }
            }
        })
        .collect()
}

/// Returns a list of surfaces that can be flattened as they are only referenced in one place.
fn find_flatten_candidates<'surface>(
    surfaces: impl IntoIterator<Item = &'surface mir::Surface>,
) -> HashSet<mir::SurfaceRef> {
    let mut visited_surfaces = HashSet::new();
    let mut flatten_candidates = HashSet::new();

    for surface in surfaces {
        for node in &surface.nodes {
            let subsurface_id = match node.data {
                mir::NodeData::Group(subsurface_id) => subsurface_id,
                _ => continue,
            };

            if visited_surfaces.insert(subsurface_id) {
                // The surface hasn't been visited yet, insert it into the candidates list.
                flatten_candidates.insert(subsurface_id);
            } else {
                // We're revisiting the surface, so remove it from the candidates in case it's there.
                flatten_candidates.remove(&subsurface_id);
            }
        }
    }

    flatten_candidates
}
