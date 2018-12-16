use crate::mir;
use std::collections::{HashMap, HashSet};

// Sorts sockets on group nodes so their internal socket mapping has a stable ordering for surface deduplication.
pub fn sort_group_sockets(surfaces: &mut HashMap<mir::SurfaceRef, mir::Surface>) {
    let mut visited_surfaces = HashSet::new();
    let mut socket_indices = HashMap::new();
    visit_surface(
        0,
        None,
        &mut visited_surfaces,
        surfaces,
        &mut socket_indices,
    );
}

// Returns a mapping of old socket indices to new ones
fn fix_socket_indices_in_surface(surface: &mut mir::Surface, socket_count: usize) -> Vec<usize> {
    let mut mapping = vec![0; socket_count];
    let mut socket_index = 0;

    for value_group in &mut surface.groups {
        let old_socket_index = match &mut value_group.source {
            mir::ValueGroupSource::Socket(socket_index) => socket_index,
            _ => continue,
        };

        mapping[*old_socket_index] = socket_index;
        *old_socket_index = socket_index;
        socket_index += 1;
    }

    mapping
}

fn visit_surface(
    surface_id: mir::SurfaceRef,
    socket_count: Option<usize>,
    visited_surfaces: &mut HashSet<mir::SurfaceRef>,
    all_surfaces: &mut HashMap<mir::SurfaceRef, mir::Surface>,
    socket_indices: &mut HashMap<mir::SurfaceRef, Vec<usize>>,
) {
    if !visited_surfaces.insert(surface_id) {
        return;
    }

    //let surface = all_surfaces.get_mut(&surface_id).unwrap();
    let mut surface = all_surfaces.remove(&surface_id).unwrap();

    if let Some(socket_count) = socket_count {
        let fixed_indices = fix_socket_indices_in_surface(&mut surface, socket_count);
        socket_indices.insert(surface_id, fixed_indices);
    }

    for node in &mut surface.nodes {
        let subsurface_id = match node.data {
            mir::NodeData::Group(surface_id) => surface_id,
            mir::NodeData::ExtractGroup { surface, .. } => surface,
            _ => continue,
        };

        visit_surface(
            subsurface_id,
            Some(node.sockets.len()),
            visited_surfaces,
            all_surfaces,
            socket_indices,
        );

        let socket_permutation = &socket_indices[&subsurface_id];

        // Re-order the sockets in the node to match the new ordering
        let mut dest_sockets = vec![None; node.sockets.len()];
        for (old_socket_index, socket) in node.sockets.drain(..).enumerate() {
            let new_socket_index = socket_permutation[old_socket_index];
            dest_sockets[new_socket_index] = Some(socket);
        }
        node.sockets
            .extend(dest_sockets.into_iter().map(|socket| socket.unwrap()));

        // Fix up source/destination sockets in extract groups
        if let mir::NodeData::ExtractGroup {
            source_sockets,
            dest_sockets,
            ..
        } = &mut node.data
        {
            for source_socket in source_sockets {
                *source_socket = socket_permutation[*source_socket];
            }
            for dest_socket in dest_sockets {
                *dest_socket = socket_permutation[*dest_socket];
            }
        }
    }

    all_surfaces.insert(surface_id, surface);
}
