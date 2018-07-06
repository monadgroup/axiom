use mir;
use std::collections::{HashMap, HashSet, VecDeque};

// groups extracted nodes into subsurfaces
pub fn group_extracted(
    surface: &mut mir::Surface,
    allocator: &mut mir::IdAllocator,
) -> Vec<mir::Surface> {
    GroupExtractor::new(surface).extract_groups(allocator)
}

type ValueGroupRef = usize;
type NodeRef = usize;
type ValueSocketRef = (NodeRef, usize);

#[derive(Debug)]
struct ExtractGroup {
    pub sources: HashSet<ValueGroupRef>,
    pub destinations: HashSet<ValueGroupRef>,
    pub nodes: Vec<NodeRef>,
    pub value_groups: Vec<ValueGroupRef>,
}

#[derive(Debug, Clone)]
struct ValueGroupData {
    pub sockets: Vec<ValueSocketRef>,
}

struct GroupExtractor<'a> {
    surface: &'a mut mir::Surface,
}

impl ExtractGroup {
    pub fn new() -> Self {
        ExtractGroup {
            sources: HashSet::new(),
            destinations: HashSet::new(),
            nodes: Vec::new(),
            value_groups: Vec::new(),
        }
    }
}

impl ValueGroupData {
    pub fn new() -> Self {
        ValueGroupData {
            sockets: Vec::new(),
        }
    }
}

impl<'a> GroupExtractor<'a> {
    pub fn new(surface: &'a mut mir::Surface) -> Self {
        GroupExtractor { surface }
    }

    pub fn extract_groups(&mut self, allocator: &mut mir::IdAllocator) -> Vec<mir::Surface> {
        self.find_extracted_groups()
            .into_iter()
            .enumerate()
            .map(|(index, group)| self.extract_group(allocator, index, group))
            .collect()
    }

    /// Here we move the extracted nodes into a new ExtractGroup node.
    /// We attempt to keep the surface structure as intact as possible: it's inevitable that
    /// we have to move nodes, but we don't remove value groups that should only exist in the
    /// group here, instead leaving orphaned ones on the surface. The "remove_dead_groups" pass
    /// will remove any groups that aren't necessary.
    fn extract_group(
        &mut self,
        allocator: &mut mir::IdAllocator,
        extract_index: usize,
        extract_group: ExtractGroup,
    ) -> mir::Surface {
        // Build up an index of value group mappings (from the old surface to the new surface).
        // Note that the groups in the new surface are indexed based on where they are in the
        // value_groups array - this makes things easier later on when we actually create the
        // groups.
        let mut value_group_read_write = Vec::new();
        let mut new_value_group_indexes = HashMap::new();
        for (internal_index, &parent_group_index) in extract_group.value_groups.iter().enumerate() {
            value_group_read_write.push((false, false));
            new_value_group_indexes.insert(parent_group_index, internal_index);
        }

        // Move nodes into the new surface, remapping value group indices as we go.
        // We also need to know whether value groups are written to/read from later on, so this
        // info is recorded here.
        let mut new_nodes = Vec::new();
        for (removed_count, &node_index) in extract_group.nodes.iter().enumerate() {
            let real_node_index = node_index - removed_count;
            let mut new_node = self.surface.nodes.remove(real_node_index);

            // update socket indices to the indices in the new surface
            for socket in new_node.sockets.iter_mut() {
                let remapped_id = new_value_group_indexes[&socket.group_id];
                socket.group_id = remapped_id;

                // update the value groups `value_written` and `value_read` flags
                if socket.value_read {
                    value_group_read_write[remapped_id].0 = true;
                }
                if socket.value_written {
                    value_group_read_write[remapped_id].1 = true;
                }
            }

            new_nodes.push(new_node)
        }

        // Build the new value groups and sockets where necessary
        let mut new_value_groups = Vec::new();
        let mut new_sockets = Vec::new();
        for (internal_index, &parent_group_index) in extract_group.value_groups.iter().enumerate() {
            // There are a few cases where we want to expose an internal value group as a socket:
            //  - If the group is one of the source or destination groups (in this case, we also
            //    need to 'unwrap' the group type from an array for our internal group)
            //  - If the group is exposed by the main surface
            //  - If the group is only read from by the internal nodes
            //    The reason for this is two-fold: "input-only" groups (where there's a
            //    non-extracted node feeding into the extract group), and to allow users to
            //    configure controls from the UI

            let (group_read, group_written) = value_group_read_write[internal_index];
            let parent_group = &self.surface.groups[parent_group_index];

            // if the group is a source or destination, unwrap and expose it
            let is_source_or_destination = extract_group.sources.contains(&parent_group_index)
                || extract_group.destinations.contains(&parent_group_index);

            let is_socket_source = if let mir::ValueGroupSource::Socket(_) = parent_group.source {
                true
            } else {
                false
            };
            if is_source_or_destination || is_socket_source || !group_written {
                let socket_index = new_sockets.len();

                // if it's a source or destination group, we want the actual array item
                let value_group_type = if is_source_or_destination {
                    parent_group.value_type.base_type().unwrap()
                } else {
                    &parent_group.value_type
                };
                new_value_groups.push(mir::ValueGroup::new(
                    value_group_type.clone(),
                    mir::ValueGroupSource::Socket(socket_index),
                ));
                new_sockets.push(mir::ValueSocket::new(
                    parent_group_index,
                    group_written,
                    group_read,
                    false,
                ));
            } else {
                new_value_groups.push(parent_group.clone());
            }
        }

        let new_surface = mir::Surface::new(
            mir::SurfaceId::new(
                format!("{}.extracted{}", self.surface.id.debug_name, extract_index),
                allocator,
            ),
            new_value_groups,
            new_nodes,
        );
        let new_node = mir::Node::new(
            new_sockets,
            mir::NodeData::ExtractGroup {
                surface: new_surface.id.id,
                source_sockets: extract_group
                    .sources
                    .into_iter()
                    .map(|group| new_value_group_indexes[&group])
                    .collect(),
                dest_sockets: extract_group
                    .destinations
                    .into_iter()
                    .map(|group| new_value_group_indexes[&group])
                    .collect(),
            },
        );

        // add the new node to the parent surface
        self.surface.nodes.push(new_node);

        new_surface
    }

    fn find_extracted_groups(&self) -> Vec<ExtractGroup> {
        let mut extract_groups = Vec::new();
        let mut value_groups = vec![ValueGroupData::new(); self.surface.groups.len()];
        let mut trace_queue = VecDeque::new();
        let mut value_group_extracts = HashMap::new();
        let mut node_extracts = HashMap::new();

        // In order to find extracted groups, we need two things:
        //  - The sockets that are 'connected' to a group, and whether that group then contains an extractor
        //  - The extract sockets, which seed our breadth first search. Extract sockets that are
        //    written to also become sources to their extract group, ones that are read from become
        //    destinations.
        // For efficiency, we can do these both in the same loop!
        for (node_index, node) in self.surface.nodes.iter().enumerate() {
            for (socket_index, socket) in node.sockets.iter().enumerate() {
                let socket_ref = (node_index, socket_index);
                let value_group = &mut value_groups[socket.group_id];
                value_group.sockets.push(socket_ref);

                if socket.is_extractor {
                    let extract_group_index =
                        if let Some(&existing_group) = value_group_extracts.get(&socket.group_id) {
                            existing_group
                        } else {
                            let group_index = extract_groups.len();
                            value_group_extracts.insert(socket.group_id, group_index);
                            extract_groups.push(ExtractGroup::new());
                            group_index
                        };
                    trace_queue.push_back(socket.group_id);

                    if socket.value_written {
                        extract_groups[extract_group_index]
                            .sources
                            .insert(socket.group_id);
                    }
                    if socket.value_read {
                        extract_groups[extract_group_index]
                            .destinations
                            .insert(socket.group_id);
                    }
                }
            }
        }

        // Now we can breadth-first search through groups.
        // This basically means looking at each node attached to a group, and either:
        //  - Spreading to any value groups from other sockets on the node _if it doesn't already have an extract group_
        //  - Merging extract groups if it does
        // We only propagate into nodes where their socket _reads_ from a group, and never propagate into extract sockets
        while let Some(value_group_index) = trace_queue.pop_front() {
            let extract_group_index = value_group_extracts[&value_group_index];

            extract_groups[extract_group_index]
                .value_groups
                .push(value_group_index);

            // look at each of the sockets connected to this value group
            for &(connected_node_index, connected_socket_index) in
                &value_groups[value_group_index].sockets
            {
                let connected_node = &self.surface.nodes[connected_node_index];
                let connected_socket = &connected_node.sockets[connected_socket_index];

                // if the node already has an extractor group, merge it with ours and stop there
                // otherwise, it becomes part of our group and we propagate to all of its value groups
                if let Some(connected_extract_group_index) =
                    node_extracts.get(&connected_node_index).cloned()
                {
                    GroupExtractor::merge_extract_groups(
                        extract_group_index,
                        connected_extract_group_index,
                        &mut extract_groups,
                        &mut value_group_extracts,
                        &mut node_extracts,
                    )
                } else if !connected_socket.is_extractor && connected_socket.value_read {
                    extract_groups[extract_group_index]
                        .nodes
                        .push(connected_node_index);
                    node_extracts.insert(connected_node_index, extract_group_index);

                    for socket in &connected_node.sockets {
                        if value_group_extracts.get(&socket.group_id).is_none() {
                            value_group_extracts.insert(socket.group_id, extract_group_index);
                            trace_queue.push_back(socket.group_id);
                        }
                    }
                }
            }
        }

        // remove any empty extract groups (with no nodes)
        extract_groups.retain(|group| !group.nodes.is_empty());
        extract_groups
    }

    fn merge_extract_groups(
        dest_index: usize,
        src_index: usize,
        groups: &mut [ExtractGroup],
        value_group_extracts: &mut HashMap<ValueGroupRef, usize>,
        node_extracts: &mut HashMap<NodeRef, usize>,
    ) {
        // don't try and merge if the groups are the same
        if dest_index == src_index {
            return;
        }

        {
            // migrate all references over to the destination group in the two HashMaps
            let src_group = &groups[src_index];
            for &source_ref in &src_group.sources {
                value_group_extracts.insert(source_ref, dest_index);
            }
            for &dest_ref in &src_group.destinations {
                value_group_extracts.insert(dest_ref, dest_index);
            }
            for &node_ref in &src_group.nodes {
                node_extracts.insert(node_ref, dest_index);
            }
        }

        {
            // concatenate source/node/group values onto destination
            let sources = groups[src_index].sources.clone();
            let destinations = groups[src_index].destinations.clone();
            let nodes = groups[src_index].nodes.clone();
            let value_groups = groups[src_index].value_groups.clone();

            let dest_group = &mut groups[dest_index];
            dest_group.sources.extend(sources);
            dest_group.destinations.extend(destinations);
            dest_group.nodes.extend(nodes);
            dest_group.value_groups.extend(value_groups);
        }

        {
            // remove all items from the source
            let src_group = &mut groups[src_index];
            src_group.sources.clear();
            src_group.destinations.clear();
            src_group.nodes.clear();
            src_group.value_groups.clear();
        }
    }
}
