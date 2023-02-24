use crate::db_helpers::db_collection::DbCollection;
use crate::db_helpers::fd::DbFd;
use crate::db_helpers::seq::DbTime;
use crate::db_helpers::{db_engine, db_functions};

pub(crate) static mut ENGINES: [DbCollection; 2] = [
    DbCollection::PgEngine(db_functions::DbFunctions::No),
    DbCollection::MyEngine(db_functions::DbFunctions::No),
];

pub(crate) static mut FD_SET: DbFd = DbFd::No;
pub(crate) static mut TIME_ENTRIES: DbTime = DbTime::No;
