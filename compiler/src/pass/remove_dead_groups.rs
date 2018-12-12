use crate::mir;
use std::collections::HashMap;

/// Remove value groups that don't have any sockets attached to them.
pub fn remove_dead_groups(surface: &mut mir::Surface) {
    let group_counts = find_group_usages(surface);

    let mut new_group_indices = HashMap::new();
    let mut removed_count = 0;
    for (group_index, usage_count) in group_counts.into_iter().enumerate() {
        if usage_count > 0 {
            new_group_indices.insert(group_index, group_index - removed_count);
        } else {
            surface.groups.remove(group_index - removed_count);
            removed_count += 1;
        }
    }

    // fix up the group ID refs in each socket
    for node in surface.nodes.iter_mut() {
        for socket in node.sockets.iter_mut() {
            socket.group_id = new_group_indices[&socket.group_id];
        }
    }
}

/// Build a list of the number of times each group is used
fn find_group_usages(surface: &mir::Surface) -> Vec<usize> {
    let mut usage_counts = vec![0; surface.groups.len()];

    for node in &surface.nodes {
        for socket in &node.sockets {
            usage_counts[socket.group_id] += 1;
        }
    }

    usage_counts
}
