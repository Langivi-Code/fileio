use std::borrow::BorrowMut;
use std::collections::HashMap;
use crate::db_helpers::db_engine::DbEngine;

pub(crate) enum DbCollection {
    PgEngine(DbEngine),
    MyEngine(DbEngine),
}

impl DbCollection {
    fn get(&mut self) -> &mut DbEngine {
        match self.borrow_mut() {
            DbCollection::PgEngine(e) | DbCollection::MyEngine(e) => e
        }
    }

    fn set(&mut self, db: DbEngine) {
        match self {
            DbCollection::PgEngine(e) | DbCollection::MyEngine(e) => {
                *e = db;
            }
        }
    }

    pub fn get_engine(collection_item: &mut DbCollection) -> &mut DbEngine {
        match collection_item.get() {
            DbEngine::No => {
                collection_item.set(DbEngine::Hash(HashMap::new()));
                collection_item.get()
            }
            DbEngine::Hash(_db_engine) => collection_item.get()
        }
    }
}
