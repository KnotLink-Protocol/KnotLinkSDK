// KnotLink SDK - Rust
// Copyright (c) 2024-2026 KnotLink Contributors
// SPDX-License-Identifier: MIT

use std::collections::HashMap;

pub type KLKVMap = HashMap<String, String>;

pub trait KvMapExt {
    fn serialize(&self) -> String;
    fn deserialize(&mut self, s: &str);
    fn get_value(&self, key: &str) -> Option<&String>;
}

impl KvMapExt for HashMap<String, String> {
    fn serialize(&self) -> String {
        self.iter()
            .map(|(k, v)| format!("{}={}", k, v))
            .collect::<Vec<_>>()
            .join(";")
    }

    fn deserialize(&mut self, s: &str) {
        self.clear();
        for pair in s.split(';') {
            let parts: Vec<&str> = pair.splitn(2, '=').collect();
            if parts.len() == 2 {
                self.insert(parts[0].to_string(), parts[1].to_string());
            }
        }
    }

    fn get_value(&self, key: &str) -> Option<&String> {
        HashMap::get(self, key)
    }
}