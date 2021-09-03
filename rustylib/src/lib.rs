#[no_mangle]

// Callable from C/C++, print a message a certain number of times
pub extern "C" fn call_me_from_c(num: i32) 
{
    let mut x = 0;
    while x < num {
        println!("Congratulations, you reached a rust function from somewhere else");
        x = x + 1;
    }
}