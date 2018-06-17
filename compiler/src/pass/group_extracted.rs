use mir;
use std::collections::{HashMap, HashSet, VecDeque};

// groups extracted nodes into subsurfaces
pub fn group_extracted(surface: &mut mir::Surface) -> Vec<mir::Surface> {
    GroupExtractor::new(surface).extract_groups()
}

type NodeRef = usize;
type ControlRef = (NodeRef, usize);

#[derive(Debug, Clone)]
struct ExtractedGroup {
    sources: Vec<ControlRef>,
    nodes: Vec<NodeRef>,
    groups: Vec<usize>,
}

struct GroupExtractor<'a> {
    surface: &'a mut mir::Surface,
    new_surfaces: Vec<mir::Surface>,
}

impl ExtractedGroup {
    pub fn new(source: ControlRef) -> Self {
        ExtractedGroup {
            sources: vec![source],
            nodes: Vec::new(),
            groups: Vec::new(),
        }
    }
}

impl<'a> GroupExtractor<'a> {
    pub fn new(surface: &'a mut mir::Surface) -> Self {
        GroupExtractor {
            surface,
            new_surfaces: Vec::new(),
        }
    }

    pub fn extract_groups(self) -> Vec<mir::Surface> {
        println!("{:#?}", self.find_extracted_groups());
        self.new_surfaces
    }

    fn find_extracted_groups(&self) -> Vec<ExtractedGroup> {
        let mut control_groups = vec![(Vec::new(), false); self.surface.groups.len()];
        let mut trace_queue: VecDeque<(ControlRef, bool)> = VecDeque::new();
        let mut visited_controls = HashSet::new();
        let mut root_extract_sources = HashMap::new();
        let mut node_extract_sources = HashMap::new();
        let mut extract_sources = Vec::new();

        // Here we floodfill to find all extracted nodes and their 'sources' (i.e the extractor
        // controls that lead to them)
        // This is a three-step process:
        //  1. Find all extractor controls that write to their group
        //  2. Breadth first search from writers to readers in control groups, marking nodes as
        //     extracted and keeping track of their 'source group'

        // find all extractor controls that write
        // since we're iterating, we also need to build up a list of which controls are in which
        // control groups, so we do that here too
        for (node_index, node) in self.surface.nodes.iter().enumerate() {
            for (control_index, control) in node.controls.iter().enumerate() {
                control_groups[control.group_id]
                    .0
                    .push((node_index, control_index));

                let control_ref = (node_index, control_index);
                if control.control_type.is_extract() {
                    trace_queue.push_back((control_ref, false));
                    visited_controls.insert((node_index, control_index));
                    control_groups[control.group_id].1 = true;

                    if control.value_written {
                        root_extract_sources.insert(control_ref, extract_sources.len());
                        extract_sources.push(ExtractedGroup::new(control_ref));
                    }
                }
            }
        }

        while let Some(((parent_node_index, control_index), scan_siblings)) =
            trace_queue.pop_front()
        {
            let parent_node = &self.surface.nodes[parent_node_index];
            let control = &parent_node.controls[control_index];

            let extract_group = node_extract_sources
                .get(&parent_node_index)
                .or_else(|| root_extract_sources.get(&(parent_node_index, control_index)))
                .cloned();

            // if the control isn't an extractor, we also want to propagate from other controls
            // on the node surface
            if scan_siblings {
                for (control_index, sibling_control) in parent_node.controls.iter().enumerate() {
                    let control_ref = (parent_node_index, control_index);
                    if visited_controls.contains(&control_ref) {
                        continue;
                    };
                    visited_controls.insert(control_ref);

                    // continue propagation if we haven't hit another extractor
                    if !sibling_control.control_type.is_extract() {
                        trace_queue.push_back((control_ref, false))
                    }
                }
            }

            // propagate to any controls in the shared group that need to be extracted
            // a control needs to be extracted if:
            //  - it is NOT an extractor, AND EITHER
            //  - it reads from the `next_control`, which we already know is extracted, OR
            //  - it writes to the group, and the group contains an extractor
            // while we propagate, we also need to either join the connecting control to our group,
            // or merge groups with ours
            let (ref connected_controls, has_extractor) = control_groups[control.group_id];
            for &(connected_node_index, connected_control_index) in connected_controls {
                let control_ref = (connected_node_index, connected_control_index);
                if visited_controls.contains(&control_ref) {
                    continue;
                };
                visited_controls.insert(control_ref);

                let connected_node = &self.surface.nodes[connected_node_index];
                let connected_control = &connected_node.controls[connected_control_index];

                if let Some(current_group) = extract_group {
                    if !has_extractor {
                        extract_sources[current_group].groups.push(control.group_id);
                    }
                }

                if !connected_control.control_type.is_extract()
                    && (connected_control.value_read || has_extractor)
                {
                    if let Some(current_group) = extract_group {
                        if let Some(&connected_extract_group) =
                            node_extract_sources.get(&connected_node_index)
                        {
                            // we need to merge the two groups
                            GroupExtractor::merge_extract_groups(
                                current_group,
                                connected_extract_group,
                                &mut extract_sources,
                                &mut root_extract_sources,
                                &mut node_extract_sources,
                            );
                        } else if !connected_control.control_type.is_extract() {
                            // the node should inherit our group (but ONLY if the node isn't an extractor!)
                            extract_sources[current_group]
                                .nodes
                                .push(connected_node_index);
                            node_extract_sources.insert(connected_node_index, current_group);
                        }
                    }

                    trace_queue.push_back((control_ref, connected_node_index != parent_node_index))
                }
            }
        }

        // we can now remove all duplicate groups
        extract_sources.retain(|group| !group.nodes.is_empty());
        extract_sources
    }

    fn merge_extract_groups(
        dest_index: usize,
        src_index: usize,
        groups: &mut [ExtractedGroup],
        root_extract_sources: &mut HashMap<ControlRef, usize>,
        node_extract_sources: &mut HashMap<NodeRef, usize>,
    ) {
        // we don't want to try and merge if they're the same
        if dest_index == src_index {
            return;
        }

        {
            // migrate all sources and nodes over to the destination group in the two HashMaps
            let src_group = &groups[src_index];
            for &source_control in &src_group.sources {
                root_extract_sources.insert(source_control, dest_index);
            }
            for &node in &src_group.nodes {
                node_extract_sources.insert(node, dest_index);
            }
        }

        // concatenate source/node values onto destination
        {
            let sources = groups[src_index].sources.to_vec();
            let nodes = groups[src_index].nodes.to_vec();
            let value_groups = groups[src_index].groups.to_vec();
            let dest_group = &mut groups[dest_index];
            dest_group.sources.extend(sources);
            dest_group.nodes.extend(nodes);
            dest_group.groups.extend(value_groups);
        }

        // remove all items from the source
        {
            let src_group = &mut groups[src_index];
            src_group.sources.clear();
            src_group.nodes.clear();
            src_group.groups.clear();
        }
    }
}
