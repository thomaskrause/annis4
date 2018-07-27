use super::conjunction::Conjunction;
use {Component};

pub struct Disjunction<'a> {
    pub alternatives : Vec<Conjunction<'a>>,
}

impl<'a> Disjunction<'a> {
    pub fn new(alternatives :Vec<Conjunction<'a>>) -> Disjunction<'a> {
        Disjunction {
            alternatives : alternatives,
        }
    }

    pub fn necessary_components(&self) -> Vec<Component> {
        let mut result = vec![];
        
        for alt in self.alternatives.iter() {
            let mut c = alt.necessary_components();
            result.append(&mut c);
        }

        return result;
    }

    pub fn get_variable_pos(&self, variable : &str) -> Option<usize> {
        for alt in self.alternatives.iter() {
            if let Some(var_pos) = alt.get_variable_pos(variable) {
                return Some(var_pos);
            }
        }
        return None;
    }
}