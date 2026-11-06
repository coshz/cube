import Foundation
// import Cube 

extension SolveResult: LocalizedError, CustomStringConvertible {
    public var description: String {
        return String(cString: c_solve_result_info(self))
    }

    public var errorDescription: String? {
        if self == .success {
            return nil
        }
        return description
    }
}

// MARK: - Core APIs

/// Initialize cube tables
/// 
/// - Parameters:
///   - useSymmetry: Select the table set:
///         `false`: smaller (<10 MB), fast to build, slower solving;
///         `true`:  symmetry-based (~260 MB), slower to build, faster solving.
///   - tableDir: Directory for reading and writting table files. 
///         nil means the default directory
///         Tables are loaded if present; otherwise they are built and written here. 
/// - Notes: 
///     Only the first call takes effect; call it before the first @ref solve,
///     which invokes it with default arguments.
/// 
/// - Warning: 
///     With `use_symmetry = true`, the first run may take minutes and write
///     ~260 MB of data to @p dir.
/// 
func cubeInit(useSymmetry: Bool = true, tableDir: String? = nil)
{
    if let dir = tableDir {
        dir.withCString { cString in 
            c_cube_init(useSymmetry, cString);
        }
    } else {
        c_cube_init(useSymmetry, nil)
    }
}

/// Solves the Rubik's cube
///
/// Uses Kociemba's two-phase algorithm. The search stops as soon as any solution 
/// of length at most `good` is found, trading optimality for speed. 
/// If the returned maneuver is *longer* than `good`, no shorter solution exists
/// and the result is the two-phase optimum. 
/// 
/// - Parameters:
///   - src: The source color configuration string
///   - tgt: The target color configuration string (nil for `cubeIdentity`)
///   - good: Acceptable solution length. Pass `0` to disable early-exit (default:`22`)
/// - Returns: A space-separated maneuver, eg. `"R U' F D' L2 B"`
/// - Throws: A ``SolveResult`` if the src/tgt configuration is invalid or the
///           cube is unsolvable
/// 
public func solve(
    src: String,
    tgt: String? = nil,
    good: Int32 = 22
) throws -> String {
    var buffer = [CChar](repeating: 0, count: Int(CUBE_BS))
    let result = c_solve(&buffer,src,tgt,good)
    guard result == .success else {
        throw result
    }
    return String(cString:buffer)
}

/// Checks the validity and solvability of a color configuration.
///
/// - Parameter cube: A 54-character color configuration string.
/// - Returns: `true` if the color configuration is valid and solvable.
public func solvable(cube: String) -> Bool {
    return c_solvable(cube)
}

/// Applies a maneuver to a cube and returns the resulting color configuration
///
/// - Parameters:
///   - maneuver: The sequence of moves to apply, eg. `"U R' F D L2 B"`
///   - cube: The initial color configuration. Defaults to the solved cube.
/// - Returns: The resulting color configuration.
///
public func facecube(maneuver: String, cube: String = String(CUBE_ID)) -> String {
    var buffer = [CChar](repeating: 0, count: Int(CUBE_BS))
    c_facecube(&buffer,maneuver,cube)
    return String(cString:buffer)
}

/// Computes the permutation representation of a given maneuver or cube
///
/// - Parameters:
///   - msOrCube: A color cube or a maneuver;
///   - fmt: Output format. One of:
///       - `0`: 54-face;   eg. "U1U2U3...B9"
///       - `1`: 20-cubie;  eg. "ABCDEFGH00000000opqrstuvwxyz000000000000"
///       - `2`: cycles:    eg. "(ufl,urf,ubr)(uf,ul,ur)(+u)(−d)"
/// - Returns: The permutation encoded as a string in the requested format
public func permutation(msOrCube: String, fmt: Int32 = 2) -> String {
    var buffer = [CChar](repeating: 0, count: Int(CUBE_BS))
    c_permutation(&buffer, msOrCube, fmt)
    return String(cString:buffer)
}
