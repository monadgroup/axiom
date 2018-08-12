use mir::{BlockRef, NodeData, Surface, SurfaceRef};
use std::collections::{HashMap, HashSet, VecDeque};
use std::iter::FromIterator;
use std::mem;

#[derive(Debug, Default)]
pub struct SurfaceDeps {
    pub depended_by: Vec<SurfaceRef>,
    pub depends_on_surfaces: Vec<SurfaceRef>,
    pub depends_on_blocks: Vec<BlockRef>,
}

#[derive(Debug, Default)]
pub struct BlockDeps {
    pub depended_by: Vec<SurfaceRef>,
}

#[derive(Debug)]
pub struct DependencyGraph {
    surfaces: HashMap<SurfaceRef, SurfaceDeps>,
    blocks: HashMap<BlockRef, BlockDeps>,
    gc_surface_candidates: HashSet<SurfaceRef>,
    gc_block_candidates: HashSet<BlockRef>,
}

impl DependencyGraph {
    pub fn new() -> Self {
        DependencyGraph {
            surfaces: HashMap::new(),
            blocks: HashMap::new(),
            gc_surface_candidates: HashSet::new(),
            gc_block_candidates: HashSet::new(),
        }
    }

    pub fn generate_surface(&mut self, surface: &Surface) {
        let depended_by = if let Some(old_deps) = self.surfaces.remove(&surface.id.id) {
            // the new surface might not depend on things the old one did - remove it from anything
            // it depends on
            //let mut surface_remove_candidates = HashSet::new();
            for depended_surface in &old_deps.depends_on_surfaces {
                let surface_depended_by =
                    &mut self.surfaces.get_mut(depended_surface).unwrap().depended_by;
                let index = surface_depended_by
                    .iter()
                    .position(|&n| n == surface.id.id)
                    .unwrap();
                surface_depended_by.remove(index);

                if surface_depended_by.is_empty() {
                    self.gc_surface_candidates.insert(*depended_surface);
                }
            }

            //let mut block_remove_candidates = HashSet::new();
            for depended_block in &old_deps.depends_on_blocks {
                let block_depended_by =
                    &mut self.blocks.get_mut(depended_block).unwrap().depended_by;
                let index = block_depended_by
                    .iter()
                    .position(|&n| n == surface.id.id)
                    .unwrap();
                block_depended_by.remove(index);

                if block_depended_by.is_empty() {
                    self.gc_block_candidates.insert(*depended_block);
                }
            }

            old_deps.depended_by
        } else {
            Vec::new()
        };

        let mut depends_on_surfaces = Vec::new();
        let mut depends_on_blocks = Vec::new();

        // build the new dep data
        for node in &surface.nodes {
            match node.data {
                NodeData::Dummy => {}
                NodeData::Custom(block) => depends_on_blocks.push(block),
                NodeData::Group(surface) => depends_on_surfaces.push(surface),
                NodeData::ExtractGroup { surface, .. } => depends_on_surfaces.push(surface),
            }
        }

        // update the depended_by field on each surface/block
        for depended_surface in &depends_on_surfaces {
            let surface_deps = self
                .surfaces
                .entry(*depended_surface)
                .or_insert_with(|| SurfaceDeps::default());
            surface_deps.depended_by.push(surface.id.id);
            self.gc_surface_candidates.remove(depended_surface);
        }
        for depended_block in &depends_on_blocks {
            let block_deps = self
                .blocks
                .entry(*depended_block)
                .or_insert_with(|| BlockDeps::default());
            block_deps.depended_by.push(surface.id.id);
            self.gc_block_candidates.remove(depended_block);
        }

        self.surfaces.insert(
            surface.id.id,
            SurfaceDeps {
                depended_by,
                depends_on_surfaces,
                depends_on_blocks,
            },
        );
    }

    pub fn get_surface_deps(&self, surface: SurfaceRef) -> Option<&SurfaceDeps> {
        self.surfaces.get(&surface)
    }

    pub fn get_block_deps(&self, block: BlockRef) -> Option<&BlockDeps> {
        self.blocks.get(&block)
    }

    pub fn is_ancestor_of_surface(&self, base: SurfaceRef, ancestor: SurfaceRef) -> bool {
        if base == ancestor {
            return false;
        }

        let mut check_surfaces = VecDeque::new();
        check_surfaces.push_back(base);

        while let Some(surface) = check_surfaces.pop_front() {
            for &parent_surface in &self.get_surface_deps(surface).unwrap().depended_by {
                if parent_surface == ancestor {
                    return true;
                }

                check_surfaces.push_back(parent_surface);
            }
        }

        false
    }

    pub fn is_descendent_of_surface(&self, base: SurfaceRef, descendent: SurfaceRef) -> bool {
        if base == descendent {
            return false;
        }

        let mut check_surfaces = VecDeque::new();
        check_surfaces.push_back(base);

        while let Some(surface) = check_surfaces.pop_front() {
            for &child_surface in &self.get_surface_deps(surface).unwrap().depends_on_surfaces {
                if child_surface == descendent {
                    return true;
                }

                check_surfaces.push_back(child_surface);
            }
        }

        false
    }

    pub fn garbage_collect(&mut self) {
        // use the surface candidates as seeds
        let mut old_surfaces = HashSet::new();
        mem::swap(&mut self.gc_surface_candidates, &mut old_surfaces);

        let mut surface_removals = VecDeque::from_iter(old_surfaces.into_iter());
        while let Some(surface) = surface_removals.pop_front() {
            // the root surface should never be able to get here
            assert_ne!(surface, 0);

            if let Some(surface_deps) = self.surfaces.remove(&surface) {
                for depend_surface in surface_deps.depends_on_surfaces {
                    // if the surface only has one depended_by (which is us), remove it
                    if self.surfaces[&depend_surface].depended_by.len() <= 1 {
                        surface_removals.push_back(depend_surface);
                    }
                }

                for depend_block in surface_deps.depends_on_blocks {
                    // if the block only has one depended_by (which is us), mark it for removal
                    // later, otherwise remove us from the list
                    let depend_block_deps =
                        &mut self.blocks.get_mut(&depend_block).unwrap().depended_by;
                    if depend_block_deps.len() <= 1 {
                        self.gc_block_candidates.insert(depend_block);
                    } else {
                        let index = depend_block_deps
                            .iter()
                            .position(|&i| i == surface)
                            .unwrap();
                        depend_block_deps.remove(index);
                    }
                }
            }
        }

        let mut old_blocks = HashSet::new();
        mem::swap(&mut self.gc_block_candidates, &mut old_blocks);

        self.blocks.retain(|id, _| old_blocks.get(id).is_none());
    }
}
