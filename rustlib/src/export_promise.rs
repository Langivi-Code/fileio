use std::borrow::BorrowMut;
use std::collections::LinkedList;
use std::ops::Deref;
use std::os::raw::c_void;
use std::ptr::addr_of_mut;
use crate::ffi::{php_var_dump, zend_object, zend_refcounted_h};
use crate::promise_helpers::promise_list::PromiseList;
use crate::promise_helpers::zend_gc_try_delref;

#[no_mangle]
pub extern "C" fn init_promise_list(initial: *mut zend_object) -> *const c_void {
    let mut list = LinkedList::<*mut zend_object>::from([initial]);
    println!("{:?}", &list);
    let pointer = &mut list as *const LinkedList<*mut zend_object>;
    let ptr = pointer as u64;
    PromiseList::get_storage().insert(ptr, list);
    unsafe {
        dbg!(initial);
        dbg!(PromiseList::get_storage().get(&ptr).unwrap().front().unwrap());
    }
    dbg!(ptr);
    dbg!(pointer);
    // std::process::exit(0);
    pointer as *const c_void
    // LinkedList<*const zend_object>
}

#[no_mangle]
pub extern "C" fn add_promise_to_list(list: *const c_void, prev: *mut zend_object, next: *mut zend_object) -> bool {
    let ptr = list as u64;
    let store = PromiseList::get_storage();
    if !store.contains_key(&ptr) {
        let linked_list = LinkedList::from([prev,next]);
        store.insert(ptr, linked_list);
    }
    let linked = store.get_mut(&ptr).unwrap();
    if let Some(prev_in_list) = linked.back() {
        if *prev_in_list == prev {
            linked.push_back(next);
            println!("added new entry ");
            return true;
        }
    }
    return false;
}

#[no_mangle]
pub extern "C" fn get_next_promise_from_list(list: *const c_void, current: *mut zend_object) -> *const zend_object {
    let addr = list as u64;
    let linked = PromiseList::get_storage().get(&addr).unwrap();
    println!("here is {:?}", &linked);
    // dbg!(&list.front().unwrap());
    let next = linked.back().unwrap();
    dbg!(next);
    println!("{:?}", linked);

    let mut should_stop = false;
    if *linked.back().unwrap() == current {
        panic!("It is last element");
    } else { //rework to loop!!
        for prev_in_list in linked {
            if *prev_in_list == current {
                println!("found current");
                should_stop = true;
                continue;
            }
            if should_stop { dbg!(*prev_in_list);
                return *prev_in_list;
            }
        }
    }
    panic!("Element not in list");
}

#[no_mangle]
pub extern "C" fn has_next_promise_from_list(list: *const c_void, current: *mut zend_object) -> bool {
    let addr = list as u64;
    let linked = PromiseList::get_storage().get(&addr).unwrap();
    println!("here is {:?}", &linked);
    // dbg!(&list.front().unwrap());
    let next = linked.back().unwrap();
    dbg!(next);
    println!("{:?}", linked);

    if *(linked.back().unwrap()) == current {
        return false;
    } else {
        for prev_in_list in linked {
            if *prev_in_list == current {
                return true;
            }
        }
    }
    return false;
}

#[no_mangle]
pub extern "C" fn move_to_another_list(source_list: *const c_void, target_list: *const c_void, from: *mut zend_object) -> bool {
    let source = source_list as u64;
    let target = target_list as u64;
    let source_linked = PromiseList::get_storage().get(&source).unwrap();
    let target_linked = PromiseList::get_storage().get_mut(&target).unwrap();
    println!("Before move source_linked{:?} target_linked:{:?}", source_linked, target_linked);
    let mut should_move = false;
    if *(source_linked.back().unwrap()) == from {
        return false;
    } else {
        for prev_in_list in source_linked {
            if *prev_in_list == from && target_linked.len() == 1 {
                should_move = true;
                continue;
            }
            if should_move  {
                target_linked.push_back(*prev_in_list);
            }
        }
        println!("After move source_linked{:?} target_linked:{:?}", source_linked, target_linked);
        return true;
    }
}

#[no_mangle]
pub extern "C" fn remove_promise_list(list: *const c_void) {
    let addr = list as u64;
    let mut linked = PromiseList::get_storage().get_mut(&addr);
    if let Some(list) = linked {
        for x in list.iter() {
            println!("removing!");
            let refer = *x;
            unsafe {
                let mut refer_ref = refer.as_mut().unwrap();
                println!("bef ref count {}", & refer_ref.gc.refcount);
                zend_gc_try_delref(&mut refer_ref.gc );
                println!(" aft ref count {}", & refer_ref.gc.refcount);
            }
        }
        dbg!(list.len());
        list.clear();
        dbg!(list.len());
        PromiseList::get_storage().remove(&addr);
    }
}