use cube_rust::{Cube, cube_init};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize library (load or build tables)
    cube_init(false,"tables/");

    // 1. Initialize a cube in standard solved state
    let mut cube = Cube::default();

    // 2. Apply moves
    cube.apply_maneuver_mut("R U F' D2").unwrap();

    // 3. Check if the scrambled state is solvable
    assert!(cube.is_solvable());

    // 4. Solve the cube safely
    match cube.solve() {
        Ok(solution) => println!("Solution: {solution}"),
        Err(err) => eprintln!("Failed to solve: {err}"),
    }

    Ok(())
}
