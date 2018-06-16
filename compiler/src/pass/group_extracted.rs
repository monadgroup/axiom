use mir;
use std::collections::{HashMap, HashSet, VecDeque};

// groups extracted nodes into subsurfaces
pub fn group_extracted(surface: &mut mir::Surface) -> Vec<mir::Surface> {
    GroupExtractor::new(surface).extract_groups()
}

#[derive(Debug, Clone)]
struct ExtractedGroup<'a> {
    sources: Vec<&'a mir::Control>,
    nodes: Vec<&'a mir::Node>,
}

struct GroupExtractor<'a> {
    surface: &'a mut mir::Surface,
    new_surfaces: Vec<mir::Surface>,
}

impl<'a> ExtractedGroup<'a> {
    pub fn new(source: &'a mir::Control) -> Self {
        ExtractedGroup {
            sources: vec![source],
            nodes: Vec::new(),
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
        let mut trace_queue: VecDeque<(Option<&mir::Node>, &mir::Control, bool)> = VecDeque::new();
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
        for node in &self.surface.nodes {
            for control in &node.controls {
                control_groups[control.group_id].0.push((node, control));

                if control.control_type.is_extract() {
                    trace_queue.push_back((None, control, false));
                    visited_controls.insert(control as *const mir::Control);
                    control_groups[control.group_id].1 = true;

                    if control.value_written {
                        root_extract_sources
                            .insert(control as *const mir::Control, extract_sources.len());
                        extract_sources.push(ExtractedGroup::new(control));
                    }
                }
            }
        }

        while let Some((control_node, next_control, scan_siblings)) = trace_queue.pop_front() {
            let extract_group = match control_node {
                Some(parent_node) => node_extract_sources.get(&(parent_node as *const mir::Node)),
                None => root_extract_sources.get(&(next_control as *const mir::Control)),
            }.cloned();

            // if the control isn't an extractor, we also want to propagate from other controls
            // on the node surface
            if scan_siblings {
                if let Some(parent_node) = control_node {
                    for sibling_control in &parent_node.controls {
                        if visited_controls.contains(&(sibling_control as *const mir::Control)) {
                            continue;
                        };
                        visited_controls.insert(sibling_control);

                        // continue propagation if we haven't hit another extractor
                        if !sibling_control.control_type.is_extract() {
                            trace_queue.push_back((Some(parent_node), sibling_control, false))
                        }
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
            let (ref connected_controls, has_extractor) = control_groups[next_control.group_id];
            for &(node_control, connected_control) in connected_controls {
                if visited_controls.contains(&(connected_control as *const mir::Control)) {
                    continue;
                };
                visited_controls.insert(connected_control);

                if !connected_control.control_type.is_extract()
                    && (connected_control.value_read || has_extractor)
                {
                    if let Some(current_group) = extract_group {
                        if let Some(&connected_extract_group) =
                            node_extract_sources.get(&(node_control as *const mir::Node))
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
                            extract_sources[current_group].nodes.push(node_control);
                            node_extract_sources
                                .insert(node_control as *const mir::Node, current_group);
                        }
                    }

                    trace_queue.push_back((
                        Some(node_control),
                        connected_control,
                        Some(node_control) != control_node,
                    ))
                }
            }
        }

        // we can now remove all duplicate groups
        extract_sources.retain(|group| !group.nodes.is_empty());
        extract_sources
    }

    fn merge_extract_groups<'b>(
        dest_index: usize,
        src_index: usize,
        groups: &mut [ExtractedGroup<'b>],
        root_extract_sources: &mut HashMap<*const mir::Control, usize>,
        node_extract_sources: &mut HashMap<*const mir::Node, usize>,
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
            let dest_group = &mut groups[dest_index];
            dest_group.sources.extend(sources);
            dest_group.nodes.extend(nodes);
        }

        // remove all items from the source
        {
            let src_group = &mut groups[src_index];
            src_group.sources.clear();
            src_group.nodes.clear();
        }
    }
}
