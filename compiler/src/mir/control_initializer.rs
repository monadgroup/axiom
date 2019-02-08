use ordered_float::OrderedFloat;
use std::{fmt, hash};

#[derive(Debug, PartialEq, Clone)]
pub struct GraphControlInitializer {
    pub curve_count: u8,
    pub start_values: Vec<f64>,
    pub end_positions: Vec<f64>,
    pub tension: Vec<f64>,
    pub states: Vec<u8>,
}

impl hash::Hash for GraphControlInitializer {
    fn hash<H: hash::Hasher>(&self, state: &mut H) {
        self.curve_count.hash(state);

        state.write_usize(self.start_values.len());
        for &start_value in &self.start_values {
            OrderedFloat(start_value).hash(state);
        }

        state.write_usize(self.end_positions.len());
        for &end_pos in &self.end_positions {
            OrderedFloat(end_pos).hash(state);
        }

        state.write_usize(self.tension.len());
        for &tension in &self.tension {
            OrderedFloat(tension).hash(state);
        }

        self.states.hash(state);
    }
}

#[derive(Debug, PartialEq, Clone, Hash)]
pub enum ControlInitializer {
    None,
    Graph(GraphControlInitializer),
}

impl ControlInitializer {
    pub fn as_graph_control(&self) -> Option<&GraphControlInitializer> {
        match self {
            ControlInitializer::Graph(val) => Some(val),
            _ => None,
        }
    }
}

impl fmt::Display for ControlInitializer {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            ControlInitializer::None => write!(f, "none"),
            ControlInitializer::Graph(initializer) => {
                write!(f, "graph ({}) {{ vals = [", initializer.curve_count)?;
                for (i, &v) in initializer.start_values.iter().enumerate() {
                    write!(f, "{}", v)?;
                    if i != initializer.start_values.len() - 1 {
                        write!(f, ", ")?;
                    }
                }
                write!(f, "], pos = [")?;
                for (i, &v) in initializer.end_positions.iter().enumerate() {
                    write!(f, "{}", v)?;
                    if i != initializer.end_positions.len() - 1 {
                        write!(f, ", ")?;
                    }
                }
                write!(f, "], tension = [")?;
                for (i, &v) in initializer.tension.iter().enumerate() {
                    write!(f, "{}", v)?;
                    if i != initializer.tension.len() - 1 {
                        write!(f, ", ")?;
                    }
                }
                write!(f, "], states = [")?;
                for (i, &v) in initializer.states.iter().enumerate() {
                    write!(f, "{}", v)?;
                    if i != initializer.states.len() - 1 {
                        write!(f, ", ")?;
                    }
                }
                write!(f, "] }}")
            }
        }
    }
}
