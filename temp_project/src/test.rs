use std::borrow::BorrowMut;
use std::collections::HashMap;

pub enum DbEngine {
    No,
    Hash(HashMap<u16, String>),
}

static mut MP: DbEngine = DbEngine::No;


impl DbEngine {
    fn unwrap(&'static mut self) -> &mut HashMap<u16, String> {
        match self {
            DbEngine::No => {
                panic!("No data present");
            }
            DbEngine::Hash(val) => { val }
        }
    }
    pub fn db_map_get(&'static mut self, k: u16) -> &'static String {
        match self.unwrap().get(&k) {
            None => { panic!("No item present") }
            Some(cb) => { cb }
        }
    }
    pub fn db_map_has(&'static mut self, k: u16) -> bool {
        self.unwrap().contains_key(&k)
    }
    pub fn db_map_add(&'static mut self, fd: u16, function_item: String) {
        self.unwrap().insert(fd, function_item);
    }
}

pub enum DbCollection {
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
            DbCollection::PgEngine(db_engine) | DbCollection::MyEngine(db_engine) => {
                *db_engine = db;
            }
        }

        pub fn get_engine(collection_item: &mut DbCollection) -> &mut DbEngine {
            unsafe {
                match collection_item.get() {
                    DbEngine::No => {
                        collection_item.set(DbEngine::Hash(HashMap::new()));
                        collection_item.get()
                    }
                    DbEngine::Hash(_db_engine) => collection_item.get()
                }
            }
        }
    }
}

pub(crate) static mut ENGINES: [DbCollection; 2] = [DbCollection::PgEngine(DbEngine::No), DbCollection::MyEngine(DbEngine::No)];