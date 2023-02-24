use crate::db_helpers::db_engine::DbEngine;
use crate::db_helpers::db_functions::DbFunctions;
use std::borrow::BorrowMut;
use std::collections::HashMap;

pub(crate) enum DbCollection {
    PgEngine(DbFunctions),
    MyEngine(DbFunctions),
}

impl DbCollection {
    fn get(&mut self) -> &mut DbFunctions {
        match self.borrow_mut() {
            DbCollection::PgEngine(e) | DbCollection::MyEngine(e) => e,
        }
    }

    fn set(&mut self, db: DbFunctions) {
        match self {
            DbCollection::PgEngine(e) | DbCollection::MyEngine(e) => {
                *e = db;
            }
        }
    }

    pub fn get_engine(collection_item: &mut DbCollection, fd: u16) -> &mut DbFunctions {
        match collection_item.get() {
            DbFunctions::No => {
                collection_item.set(DbFunctions::Hash(HashMap::from([(
                    fd,
                    DbEngine::Hash(HashMap::new()),
                )])));
                collection_item.get()
            }
            DbFunctions::Hash(_db_engine) => collection_item.get(),
        }
    }
}
