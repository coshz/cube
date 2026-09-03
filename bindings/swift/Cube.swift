import Foundation
// import Cube 

extension SolveResult: LocalizedError, CustomStringConvertible {
    public var description: String {
        return String(cString: c_solve_result_to_string(self))
    }

    public var errorDescription: String? {
        if self == .success {
            return nil
        }
        return description
    }
}

/// MARK: - Core APIs

/// Solves the Rubik's cube from a source configuration to a target configuration.
///
/// - Parameters:
///   - src: The source color configuration string (nil for `cubeIdentity`);
///   - tgt: The target color configuration string (nil for `cubeIdentity`);
///   - steps: The maximum search depth (default: `30`);
///   - best: Whether to search for the shortest solution (default: `false`);
/// - Returns: A space-separated maneuver sequence solving the cube;
/// - Throws: A ``SolveResult`` error.
public func solve(
    src: String,
    tgt: String? = nil,
    steps: Int32 = 30,
    best: Bool = false
) throws -> String {
    var buffer = [CChar](repeating: 0, count: Int(CUBE_BS))
    let result = c_solve_ultimate(src,tgt,&buffer,steps,best,1)
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

/// Transforms a cube's color configuration by applying a sequence of moves.
///
/// - Parameters:
///   - cube: The initial color configuration string;
///   - maneuver: The sequence of moves to execute;12
/// - Returns: The resulting color configuration.
public func facecube(cube: String, maneuver: String) -> String {
    var buffer = [CChar](repeating: 0, count: Int(CUBE_BS))
    c_facecube(cube,maneuver,&buffer)
    return String(cString:buffer)
}

/// Computes the cycle-decomposed cubie permutation for a given move sequence.
///
/// - Parameter maneuver: The sequence of moves to analyze;
/// - Returns: The cycle decomposition representation string.
public func permutation(maneuver: String) -> String {
    var buffer = [CChar](repeating: 0, count: Int(CUBE_BS))
    c_permutation(maneuver,&buffer)
    return String(cString:buffer)
}