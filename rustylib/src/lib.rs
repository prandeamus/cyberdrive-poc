#![crate_type="staticlib"]
#![no_std]

// Callable from C/C++
#[no_mangle]
pub extern "C" fn call_me_from_c(num: i32) -> i32
{
    return num*2;
}
