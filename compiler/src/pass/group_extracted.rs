use mir;
use std::collections::{HashMap, VecDeque};

// groups extracted nodes into subsurfaces
pub fn group_extracted(surface: &mut mir::Surface) -> Vec<mir::Surface> {
    GroupExtractor::new(surface).extract_groups()
}

type ValueGroupRef = usize;
type NodeRef = usize;
type ValueSocketRef = (NodeRef, usize);

#[derive(Debug)]
struct ExtractGroup {
    pub sources: Vec<ValueGroupRef>,
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
    pub fn new(source: ValueGroupRef) -> Self {
        ExtractGroup {
            sources: vec![source],
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

    pub fn extract_groups(&mut self) -> Vec<mir::Surface> {
        println!("{:#?}", self.find_extracted_groups());

        Vec::new()
    }

    fn find_extracted_groups(&self) -> Vec<ExtractGroup> {
        let mut extract_groups = Vec::new();
        let mut value_groups = vec![ValueGroupData::new(); self.surface.groups.len()];
        let mut trace_queue = VecDeque::new();
        let mut value_group_extracts = HashMap::new();
        let mut node_extracts = HashMap::new();

        // In order to find extracted groups, we need two things:
        //  - The sockets that are 'connected' to a group, and whether that group then contains an extractor
        //  - The extractor sockets that write to their output, which seed our breadth-first search and are
        //    used as "sources" to each extract group
        // For efficiency, we can do these both in the same loop!
        for (node_index, node) in self.surface.nodes.iter().enumerate() {
            for (socket_index, socket) in node.sockets.iter().enumerate() {
                let socket_ref = (node_index, socket_index);
                let value_group = &mut value_groups[socket.group_id];
                value_group.sockets.push(socket_ref);

                if socket.is_extractor && socket.value_written
                    && value_group_extracts.get(&socket.group_id).is_none()
                {
                    let extract_group_index = extract_groups.len();
                    println!(
                        "Extractor on value-group {} has extract group {}",
                        socket.group_id, extract_group_index
                    );
                    value_group_extracts.insert(socket.group_id, extract_group_index);
                    extract_groups.push(ExtractGroup::new(socket.group_id));
                    trace_queue.push_back(socket.group_id);
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
            println!(
                "Checking out value group {}, linked to extract group {}",
                value_group_index, extract_group_index
            );

            extract_groups[extract_group_index]
                .value_groups
                .push(value_group_index);

            // look at each of the sockets connected to this value group
            for &(connected_node_index, connected_socket_index) in
                &value_groups[value_group_index].sockets
            {
                let connected_node = &self.surface.nodes[connected_node_index];
                let connected_socket = &connected_node.sockets[connected_socket_index];

                // skip if it's an extractor socket or doesn't read from the value group
                if connected_socket.is_extractor || !connected_socket.value_read {
                    continue;
                }

                println!(
                    "Spreading to node {} through socket {}",
                    connected_node_index, connected_socket_index
                );

                // if the node already has an extractor group, merge it with ours and stop there
                // otherwise, it becomes part of our group and we propagate to all of its value groups
                if let Some(connected_extract_group_index) =
                    node_extracts.get(&connected_node_index).cloned()
                {
                    println!(
                        "Merging groups {} and {}",
                        extract_group_index, connected_extract_group_index
                    );
                    GroupExtractor::merge_extract_groups(
                        extract_group_index,
                        connected_extract_group_index,
                        &mut extract_groups,
                        &mut value_group_extracts,
                        &mut node_extracts,
                    )
                } else {
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
            for &node_ref in &src_group.nodes {
                node_extracts.insert(node_ref, dest_index);
            }
        }

        {
            // concatenate source/node/group values onto destination
            let sources = groups[src_index].sources.to_vec();
            let nodes = groups[src_index].nodes.to_vec();
            let value_groups = groups[src_index].value_groups.to_vec();

            let dest_group = &mut groups[dest_index];
            dest_group.sources.extend(sources);
            dest_group.nodes.extend(nodes);
            dest_group.value_groups.extend(value_groups);
        }

        {
            // remove all items from the source
            let src_group = &mut groups[src_index];
            src_group.sources.clear();
            src_group.nodes.clear();
            src_group.value_groups.clear();
        }
    }
}
