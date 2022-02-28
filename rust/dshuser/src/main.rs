use std::io::*;
use std::fs::File;
use std::ffi::{CString};

#[link(name="dsh", kind="static")]
extern "C" {
    fn dsh_init() -> usize;
    fn dsh_run(cmd: *const i8, session: usize, command: usize);
}

fn main() -> Result<()>{
    unsafe {
        let session = dsh_init();

        let reader = BufReader::new(stdin());
        print!("dsh> ");
        stdout().flush()?;        
    
        let mut command=0;
        for line in reader.lines() {
            let cmd = CString::new(line?+"\n").unwrap();
            dsh_run(cmd.as_ptr(), session, command);
        
            print!("dsh> ");
            stdout().flush()?; 
            command += 1;       
        }
        Ok(())
    }
}