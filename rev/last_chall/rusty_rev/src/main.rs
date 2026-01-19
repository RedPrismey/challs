//INSASH{D0N7_637_rU57Y}
use rand::{thread_rng, Rng};
use std::fs::File;
use std::io::{self, Read, Write};
use std::process::exit;

fn main() {
    let roll = if is_debugger_present() {
        let mut rng = thread_rng();
        rng.gen_range(1..5)
    } else {
        0
    };

    let key = maybe_crash1(roll).bytes();

    maybe_crash2(roll);
    print!("Please input your password : ");
    std::io::stdout().flush().unwrap();

    let mut guess = String::new();

    maybe_crash3(roll);

    io::stdin().read_line(&mut guess).expect("");

    let out: Vec<u8> = guess
        .trim()
        .bytes()
        .zip(key.cycle())
        .map(|(x1, x2)| calculate(x1, x2))
        .collect();
    //(x1.rotate_left(3) ^ x1) ^ (x2.rotate_right(1) ^ x2)

    maybe_crash4(roll);

    //INSASH{D0N7_637_rU57Y}
    if out
        == [
            172, 144, 99, 101, 34, 160, 198, 201, 29, 150, 160, 78, 45, 204, 33, 9, 75, 209, 119,
            36, 245, 57,
        ]
    {
        println!("ggwp, you can validate using this password");
    } else {
        println!("Wrong password, reporting incident to the admin");
    }
}

fn calculate(x1: u8, x2: u8) -> u8 {
    maybe_crash3(2);
    let part1 = operation1(x1);
    let part2 = operation2(x2);

    part1 ^ part2
}

fn operation1(x: u8) -> u8 {
    let useless = "INSASH{pas en plain text quand même mdrrr}".bytes();
    let _bait1 = x.rotate_right(5);
    let mut right_one = x.rotate_left(3);

    right_one = right_one.rotate_right(0);
    let _useless = useless.into_iter().map(|a| a ^ x);

    right_one ^= x;

    maybe_crash1(1);

    right_one
}

fn operation2(x: u8) -> u8 {
    maybe_crash2(3);

    let a = [[31, 62], [1, 7]];
    let b = [[3, 47], [8, 17]];
    let mut out = x;
    let mut result = [[0, 0], [0, 0]];

    for i in 0..2 {
        if i == 1 {
            out = out.rotate_right(1)
        }
        for j in 0..2 {
            for k in 0..2 {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }

    out ^= x;

    out
}

fn is_debugger_present() -> bool {
    let mut file = File::open("/proc/self/status").expect("");
    let mut contents = String::new();

    file.read_to_string(&mut contents).expect("");

    let mut tracer_status = "";

    for line in contents.lines() {
        if line.contains("TracerPid:") {
            tracer_status = line.split("	").last().expect("");
        }
    }

    match tracer_status {
        "0" => false,
        x if x.parse::<u32>().is_ok() => true,
        _ => exit(0),
    }
}

fn maybe_crash1<'a>(roll: u8) -> &'a str {
    if roll == 1 {
        "ST0P17"
    } else {
        "5734M3D"
    }
}

fn maybe_crash2(roll: u8) {
    if roll != 2 {
    } else {
        println!(
            "
░░░░░░░░░░░░░░░░░█████░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░████░░░░░███░░░░░░░░░░░░░░░
░░░░██████████░░░░░░████████████░░░░░░░░
░░░░█░░░░░░░█░░░░████░░░░░░░░░░██░░░░░░░
░░░██░░░░░░██░░░░░█░░░░░░░░░░░██░░░░░░░░
░░░█░░░░░░░█░░░░░░███░░░░░█████░░░░░░░░░
░░░░█░░░░░░█░░░░░░░░░███████░░░░░░░░░░░░
░░░░░███████░░░░░░░░░░░░░░░█░░░░░░░░░░░░
░░░░░░░░░░░█░░░███████░░░░░█░░░░░░░░░░░░
░░░░░░░░░░░█░░░█░░░░░█░░░░░█░░░░░░░░░░░░
░░░░░░░░░░░█░░░█░░░░░█░░░░█░░░░░░░░░░░░░
░░░░░░░░░░░█░░░█░░░░░█░░░░█░░░░░░░░░░░░░
░░░░░░░░░░░█░░░█░░░░░█░░░░█░░░░░░░░░░░░░
░░░░░░░░░░░█░░░█░░░░░██████░░░░░░░░░░░░░
░░░░░░░░░░░█████░░░░░░░░░░░░░░░░░░░░░░░░
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░"
        );
        exit(0);
    }
}

fn maybe_crash3(roll: u8) {
    if roll == 3 {
        println!("\n[!] gdb error, please reboot the computer");
        exit(0);
    }
}

fn maybe_crash4(roll: u8) {
    if roll == 4 {
        println!("ggwp, you can validate using this password");
        exit(0)
    }
}
