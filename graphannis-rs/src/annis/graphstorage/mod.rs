use std;
use annis::{AnnoKey, Annotation, Edge, NodeID};

pub trait EdgeContainer {
    fn get_outgoing_edges(&self, source: &NodeID) -> Vec<NodeID>;
}

pub trait ReadableGraphStorage: EdgeContainer {
    fn find_connected<'a>(
        &'a self,
        source: &NodeID,
        min_distance: usize,
        max_distance: usize,
    ) -> Box<Iterator<Item = NodeID> + 'a>;
    fn distance(&self, source: &NodeID, target: &NodeID) -> Option<usize>;
    fn is_connected(&self, source: &NodeID, target: &NodeID, min_distance: usize, max_distance: usize) -> bool;
}

pub trait WriteableGraphStorage: ReadableGraphStorage {
    fn add_edge(&mut self, edge: Edge);
    fn add_edge_annotation(&mut self, edge: Edge, anno: Annotation);

    fn delete_edge(&mut self, edge: &Edge);
    fn delete_edge_annotation(&mut self, edge: &Edge, anno_key: &AnnoKey);
    fn delete_node(&mut self, node: &NodeID);
}

pub mod adjacencylist;
pub mod registry;
