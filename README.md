# cyberdrive-poc

An experiment/challenge "against the clock" to see if could interface C/C++ Rust with a view to implementing (ultimately) Windows file system filter driver in Rust.

## Summary -

Learned rust stuff, but  of time to complete the challenge, may return to this task in future.

## What happened
Took a quick look at the documentation for file system filter drivers, which is not something I've worked on before. Interface is defined in C terms with callback functions, and I know enough in general terms about the extern "C" linkage mechanisms to know I could probably get something done in C++, however it also struck me that I have only one usable Windows platform at home for development and testing, and the thought of trashing my C: drive because of a badly written filter was not appealing. No doubt in a better equipped shop you'd use something like VM images for the initial stages? So I decided to look at the rust side of things and see what could be developed in Rust or a rust/C combo just from the windows command line.

## Start with a C++ console app
I did knock up a skeleton file system watcher in C++, which is in the "justc" directory. I am aware that the calls I was using are not file system filters, but a higher-level abstration. It doesn't fulfill the original brief but it sets a baseline for some simple file-related checking. After that had been done, I decided to see if I could write some code in Rust that was callable from the C++ app. That doesn't work yet, and I'm struggling to get the linker working. So I decided to pause, write up what I learned, and make this available.

## Link to a simple function written in a rust library

C++ code has an external dependency on a function that takes a 32-bit integer and returns a 32-bit value, hence. 

```C++
extern "C" {
    int32_t call_me_from_c(int32_t num);
}
```

Declaring this as extern "C" turns off any c++ compiler name mangling.

As an aside, I was using the MSVC 64 bit compiler, although a lot of online examples refer to MINGW compiler. Logically, I suppose I should use clang so that C/C++ and Rust share the same backend, but that was the path I chose.

I attempted to create a static library in Rust as follows

Cargo new rustylib --lib

```
[package]
name = "rustylib"
version = "0.1.0"
edition = "2018"
[dependencies]
```

and a lib.rs file

```rust
#![crate_type="staticlib"]
#![no_std]

// Callable from C/C++
#[no_mangle]
pub extern "C" fn call_me_from_c(num: i32) -> i32
{
    return num*2;
}
```

I'm fairly sure this is more or less correct. The function is declares as extern "C" and with no_mangle. So it should present a conventional C view to the C++ linker. To cut a long story short, there's no universally agreed C++ calling convention or name mangling, so we go via C.

This does compile. It generates a librustlib.rlib file in the target directory. Is this correct for a static library? 

I can see the structure of the generated code - using objdump -disasm:nobytes on the rlib file. So yes, it's a static library.

```asm
call_me_from_c:
  0000000000000000: sub         rsp,28h
  0000000000000004: mov         dword ptr [rsp+24h],ecx
  0000000000000008: mov         eax,2
  000000000000000D: imul        ecx,eax
  0000000000000010: mov         dword ptr [rsp+20h],ecx
  0000000000000014: seto        al
  0000000000000017: test        al,1
  0000000000000019: jne         0000000000000024
  000000000000001B: mov         eax,dword ptr [rsp+20h]
  000000000000001F: add         rsp,28h
  0000000000000023: ret
  0000000000000024: lea         rcx,[str.0]
  000000000000002B: lea         r8,[__unnamed_1]
  0000000000000032: mov         edx,21h
  0000000000000037: call        _ZN4core9panicking5panic17h1d8cc1e1ed58a9c7E
  000000000000003C: ud2
```
Looks vaguely correct, but we'll come back to those last few lines later.

When I first tried linking I realsed I was using the 32-bit C++ compiler and had to swicth to 64 bit. Fun times. When I rebuilt the C++ in 64 bit mode, there was a linker error because _ZN4core9panicking5panic17h1d8cc1e1ed58a9c7E isn't defined.

The actual multiplication code is clear enough, but the panic code is present. I suppose this is the mangled version of core::panicking::panic from the rust runtime. I has guesses that ```rust #![no_std] ``` would take care of that, but clearly not. The app won't link because that panic handler doesn't exist in the C++ runtime (fair enough) and it's not in the rlib. Maybe I have to know the location of the rust runtime...

There are clues here: https://docs.rust-embedded.org/book/start/panicking.html which is of course a more extreme case of rust with no operating system at all, but both this task and embedded code will often rely on controlling panic behaviour because the ability to panic is a strong part of the stdlib design - I think!

Looking at https://doc.rust-lang.org/core/ this looks promising: 
> The Rust Core Library is the dependency-free1 foundation of 
> The Rust Standard Library. It is the portable glue between the language 
> and its libraries, defining the intrinsic and primitive building blocks of
> all Rust code. It links to no upstream libraries, no system libraries,
> and no libc.

## TODO 

Lots!

1. Understand and fix linker problems, confirm that i32 can be exchanged both ways.
2. Look at more complex parameters link pointers to const strings and points to mutable structs. Seems that bindgen can process windows and other C header files to generate equivalent structs, with ```rust #[repr(C)] ``` to ensure compatible memory layouts.
3. Get a proper understanding of rust memory management and borrowing, which I am stil working through
4. Look at how much of that code is inherently "unsafe" and how it could be designed or wrappered to make the code good idiomatic rust to make best use of the memory mananger
5. Once these things are much better understood, return to the original brief of getting this inside a file system filter.

Also, I mocked this up at speed using Visual Studio Code with C++ and Rust extensions, so there are few hard coded directories etc. that I'd seek to avoid in production code.
