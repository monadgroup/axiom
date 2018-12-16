use crate::mir;
use std::collections::hash_map::Entry;
use std::collections::{HashMap, HashSet};

pub fn deduplicate_surfaces(surfaces: &mut HashMap<mir::SurfaceRef, mir::Surface>) {
    // Surface ordering starts at the lowest surface in the tree, and ends at the root.
    let (surface_ordering, mut surface_dependents) =
        determine_surface_order_and_dependents(surfaces);

    // No point in processing the first surface in the list, since it's got nothing to deduplicate against
    for (ordering_index, surface_ref) in surface_ordering.iter().enumerate().skip(1) {
        let did_remove_surface = deduplicate_surface(
            *surface_ref,
            &surface_ordering[..ordering_index],
            surfaces,
            &surface_dependents,
        );

        if did_remove_surface {
            // Remove the surface from the map and dependents list too
            surfaces.remove(surface_ref);
            surface_dependents.remove(surface_ref);
        }
    }
}

// Perform a depth-first sort to determine a surface order (since we already know a good starting
// point). Todo: is there a risk of overflowing the stack? If not, we can probably use this in the
// dependency graph too.
fn determine_surface_order_and_dependents(
    surfaces: &HashMap<mir::SurfaceRef, mir::Surface>,
) -> (
    Vec<mir::SurfaceRef>,
    HashMap<mir::SurfaceRef, HashSet<mir::SurfaceRef>>,
) {
    let mut sorted_surfaces = Vec::new();
    let mut visited_surfaces = HashSet::new();
    let mut surface_dependents = HashMap::new();

    visit_surface(
        0,
        &mut visited_surfaces,
        surfaces,
        &mut sorted_surfaces,
        &mut surface_dependents,
    );

    (sorted_surfaces, surface_dependents)
}

fn visit_surface(
    surface_id: mir::SurfaceRef,
    visited_surfaces: &mut HashSet<mir::SurfaceRef>,
    all_surfaces: &HashMap<mir::SurfaceRef, mir::Surface>,
    surface_order: &mut Vec<mir::SurfaceRef>,
    surface_dependents: &mut HashMap<mir::SurfaceRef, HashSet<mir::SurfaceRef>>,
) {
    if !visited_surfaces.insert(surface_id) {
        return;
    }

    // find all surfaces this one depends on
    let surface = &all_surfaces[&surface_id];
    for node in &surface.nodes {
        let subsurface_id = match node.data {
            mir::NodeData::Group(surface_id) => surface_id,
            mir::NodeData::ExtractGroup { surface, .. } => surface,
            _ => continue,
        };

        // register the current surface as a dependent of the surface we're about to visit
        match surface_dependents.entry(subsurface_id) {
            Entry::Occupied(mut o) => {
                o.get_mut().insert(surface_id);
            }
            Entry::Vacant(v) => {
                let mut new_set = HashSet::new();
                new_set.insert(surface_id);
                v.insert(new_set);
            }
        }

        visit_surface(
            subsurface_id,
            visited_surfaces,
            all_surfaces,
            surface_order,
            surface_dependents,
        );
    }

    surface_order.push(surface_id);
}

fn are_surfaces_equivalent(lhs: &mir::Surface, rhs: &mir::Surface) -> bool {
    lhs.groups == rhs.groups && lhs.nodes == rhs.nodes
}

fn find_duplicate_surface(
    surface: &mir::Surface,
    below_surfaces: &[mir::SurfaceRef],
    all_surfaces: &HashMap<mir::SurfaceRef, mir::Surface>,
) -> Option<mir::SurfaceRef> {
    for below_surface_id in below_surfaces.iter() {
        let below_surface = match all_surfaces.get(below_surface_id) {
            Some(surface) => surface,

            // If None, the surface has been deleted since it was a duplicate, so the ID is no longer valid. We can just skip this one safely.
            None => continue,
        };

        if are_surfaces_equivalent(surface, below_surface) {
            return Some(*below_surface_id);
        }
    }
    None
}

// Returns true if the surface is deduplicated (and hence is removed), false if there were no surfaces to replace it with
fn deduplicate_surface(
    surface_id: mir::SurfaceRef,
    below_surfaces: &[mir::SurfaceRef],
    all_surfaces: &mut HashMap<mir::SurfaceRef, mir::Surface>,
    surface_dependents: &HashMap<mir::SurfaceRef, HashSet<mir::SurfaceRef>>,
) -> bool {
    let current_surface = &all_surfaces[&surface_id];

    // Determine if we are duplicated by any surfaces below us in the ordering - if not, we don't
    // need to do anything for this surface.
    let duplicate_surface_id =
        match find_duplicate_surface(current_surface, below_surfaces, all_surfaces) {
            Some(duplicate) => duplicate,
            None => return false,
        };

    // Update references in surfaces that depend on us
    let dependent_surfaces = &surface_dependents[&surface_id];
    for dependent_surface_id in dependent_surfaces {
        let dependent_surface = all_surfaces.get_mut(&dependent_surface_id).unwrap();

        // find all places where our current surface is mentioned, and swap them with the duplicate surface ID
        for node in &mut dependent_surface.nodes {
            let node_surface_id = match &mut node.data {
                mir::NodeData::Group(surface_id) => surface_id,
                mir::NodeData::ExtractGroup { surface, .. } => surface,
                _ => continue,
            };
            *node_surface_id = duplicate_surface_id;
        }
    }

    return true;
}
