use crate::db_helpers::db_collection::DbCollection;
use crate::db_helpers::db_engine;
use crate::db_helpers::fd::DbFd;

pub(crate) static mut ENGINES: [DbCollection; 2] = [DbCollection::PgEngine(db_engine::DbEngine::No), DbCollection::MyEngine(db_engine::DbEngine::No)];

pub(crate) static mut FD_SET: DbFd = DbFd::No;
