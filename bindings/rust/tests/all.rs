use cube_rust::{permutation, Cube, CubeSolver};

#[test]
fn test_cube_solvability() {
    let solved = Cube::default();
    assert!(solved.is_solvable());
    let _fake_cube = match Cube::new("R U R' U' R U R' U'") {
        Ok(cube) => cube,
        Err(err) => { eprintln!("{err}"); return; }
    };
}

#[test]
fn test_cube_permutation() {
    let scramble = "U F U' L2 R L' D2 B";
    let perm_str = permutation(scramble,2);
    assert!(!perm_str.is_empty());
}

#[test]
fn test_cube_solver() {
    let scramble = "U F U' L2 R L' D2 B";
    let scrambled_cube = Cube::default().apply_maneuver(scramble).unwrap();

    let solution = CubeSolver::new()
        .src(scrambled_cube.clone())
        .solve()
        .expect("Solution founded!");

    assert_eq!(scrambled_cube.apply_maneuver(solution).unwrap(), Cube::default());
}

