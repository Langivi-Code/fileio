use std::collections::LinkedList;
use std::os::raw::c_void;
use crate::ffi::zend_object;

fn cast_lo_list<'a>(list: *const c_void) -> &'a mut LinkedList<*const zend_object> {
    let linked = list as *mut LinkedList<*const zend_object>;
    unsafe {
        linked.as_mut().unwrap()
    }
}

#[no_mangle]
pub extern "C" fn init_promise_list(initial: *const zend_object) -> *const c_void {
    let mut list = Box::new(LinkedList::<*const zend_object>::from([initial]));
    let pointer = list.as_ref() as *const LinkedList<*const zend_object>;
    pointer as *const c_void
    // LinkedList<*const zend_object>
}

#[no_mangle]
pub extern "C" fn add_promise_to_list(list: *const c_void, prev: *const zend_object, next: *const zend_object) -> bool {
    let linked = cast_lo_list(list);
    if let Some(prev_in_list) = linked.front() {
        if *prev_in_list == prev {
            linked.push_front(next);
            return true;
        }
    }
    return false;
}

#[no_mangle]
pub extern "C" fn get_next_promise_from_list(list: *const c_void, current: *const zend_object) -> *const zend_object {
    let linked = cast_lo_list(list);
    let mut should_stop = false;
    if *linked.front().unwrap() == current {
        panic!("It is last element");
    } else {
        for prev_in_list in linked.iter() {
            if *prev_in_list == current {
                should_stop = true;
            }
            if should_stop { return *prev_in_list; }
        }
    }
    panic!("Element not in list");
}

#[no_mangle]
pub extern "C" fn has_next_promise_from_list(list: *const c_void, current: *const zend_object) -> bool {
    let linked = cast_lo_list(list);
    if *linked.front().unwrap() == current {
        return false;
    } else {
        for prev_in_list in linked.iter() {
            if *prev_in_list == current {
                return true;
            }
        }
    }
    return false;
}

#[no_mangle]
pub extern "C" fn remove_promise_list(list: *const c_void) {
    let linked = cast_lo_list(list);
    linked.clear();
}