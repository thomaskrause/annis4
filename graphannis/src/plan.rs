use {Match, AnnoKey, NodeID};
use graphdb::GraphDB;
use query::conjunction;
use query::disjunction::Disjunction;
use query::Config;
use exec::{Desc, ExecutionNode};
use std;
use std::fmt::Formatter;
use std::collections::HashSet;

#[derive(Debug)]
pub enum Error {
    ImpossibleSearch(Vec<conjunction::Error>),
}

pub struct ExecutionPlan<'a> {
    plans: Vec<Box<ExecutionNode<Item = Vec<Match>> + 'a>>,
    current_plan: usize,
    descriptions: Vec<Option<Desc>>,
    proxy_mode: bool,
    unique_result_set: HashSet<Vec<(NodeID, AnnoKey)>>,
}

impl<'a> ExecutionPlan<'a> {
    pub fn from_disjunction(
        query: &'a Disjunction<'a>,
        db: &'a GraphDB,
        config : Config,
    ) -> Result<ExecutionPlan<'a>, Error> {
        let mut plans: Vec<Box<ExecutionNode<Item = Vec<Match>> + 'a>> = Vec::new();
        let mut descriptions: Vec<Option<Desc>> = Vec::new();
        let mut errors: Vec<conjunction::Error> = Vec::new();
        for alt in query.alternatives.iter() {
            let p = alt.make_exec_node(db, &config);
            if let Ok(p) = p {
                descriptions.push(p.get_desc().cloned());
                plans.push(p);
            } else if let Err(e) = p {
                errors.push(e);
            }
        }

        if plans.is_empty() {
            return Err(Error::ImpossibleSearch(errors));
        } else {
            return Ok(ExecutionPlan {
                current_plan: 0,
                descriptions,
                proxy_mode: plans.len() == 1,
                plans,
                unique_result_set: HashSet::new(),
            });
        }
    }
}

impl<'a> std::fmt::Display for ExecutionPlan<'a> {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        for (i,d) in self.descriptions.iter().enumerate() {
            if i > 0 {
                write!(f, "---[OR]---\n")?;
            }
            if let &Some(ref d) = d {
                write!(f, "{}", d.debug_string(""))?;
            } else {
                write!(f, "<no description>")?;
            }
        }
        Ok(())
    }
}

impl<'a> Iterator for ExecutionPlan<'a> {
    type Item = Vec<Match>;

    fn next(&mut self) -> Option<Vec<Match>> {
        let n = if self.proxy_mode {
            // just act as an proxy
            self.plans[0].next()
        } else {
            let mut n = None;
            while self.current_plan < self.plans.len() {
                n = self.plans[self.current_plan].next();
                if let Some(ref res) = n {
                    // check if we already outputted this result
                    let key : Vec<(NodeID, AnnoKey)> = res.iter().map(|m : &Match|(m.node, m.anno.key.clone())).collect();
                    if self.unique_result_set.insert(key) {
                        // new result found, break out of while-loop and return the result
                        break;
                    }

                } else {
                    // proceed to next plan
                    self.current_plan += 1;
                }
            }
            n
        };

        if let Some(tmp) = n {
            if let Some(ref desc) = self.descriptions[self.current_plan] {
                let desc: &Desc = desc;
                // re-order the matched nodes by the original node position of the query
                let mut result: Vec<Match> = Vec::new();
                result.reserve(tmp.len());
                for i in 0..tmp.len() {
                    if let Some(mapped_pos) = desc.node_pos.get(&i) {
                        result.push(tmp[mapped_pos.clone()].clone());
                    } else {
                        result.push(tmp[i].clone());
                    }
                }
                return Some(result);
            } else {
                return Some(tmp);
            }
        } else {
            return None;
        }
    }
}
