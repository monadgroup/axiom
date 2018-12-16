use crate::mir;
use std::collections::hash_map::Entry;
use std::collections::HashMap;
use std::mem;

// Sorts value groups so we have a stable ordering for surface deduplication.
pub fn sort_value_groups(surface: &mut mir::Surface) {
    // Build permutation indices by going in the order that they're referenced in node sockets
    let mut visited_value_groups = HashMap::new();
    let mut permutation_indices = Vec::new();
    for node in &mut surface.nodes {
        for socket in &mut node.sockets {
            match visited_value_groups.entry(socket.group_id) {
                Entry::Occupied(o) => {
                    socket.group_id = *o.get();
                }
                Entry::Vacant(v) => {
                    let new_group_id = permutation_indices.len();
                    permutation_indices.push(socket.group_id);
                    v.insert(new_group_id);
                    socket.group_id = new_group_id;
                }
            }
        }
    }

    assert_eq!(permutation_indices.len(), surface.groups.len());

    // Apply the sort to the surfaces groups
    let mut source_groups: Vec<_> = surface.groups.drain(..).map(Some).collect();
    surface
        .groups
        .extend(permutation_indices.into_iter().map(|source_index| {
            let mut val = None;
            mem::swap(&mut val, &mut source_groups[source_index]);
            val.unwrap()
        }));
}
