use cube_rust::{permutation, Cube, CubeSolver};

#[test]
fn test_cube_solvability() {
    let solved = Cube::default();
    assert!(solved.is_solvable());
    let fake_cube = Cube::new("R U R' U' R U R' U'");
    assert!(!fake_cube.is_solvable());
}

#[test]
fn test_cube_permutation() {
    let scramble = "U F U' L2 R L' D2 B";
    let perm_str = permutation(scramble);
    assert!(!perm_str.is_empty());
}

#[test]
fn test_cube_solver() {
    let scramble = "U F U' L2 R L' D2 B";
    let scrambled_cube = Cube::default().apply_maneuver(scramble);

    let solution = CubeSolver::new()
        .src(scrambled_cube.clone())
        .solve()
        .expect("Solution founded!");

    assert_eq!(scrambled_cube.apply_maneuver(solution), Cube::default());
}

