import { createNodeCubeApi } from '@coshz/jscube';

async function main() {
    const cube = await createNodeCubeApi();

    // Scramble & check legality
    const scrambled = cube.facecube("U F U' L2 R L' D2 B")
    console.log("scrambled: ", scrambled);
    console.log("solvable:", cube.solvable(scrambled));

    // useSymmetry=true: faster solving, but ~260 MB of tables. 
    // Build once on first run (a few minutes), then cached via TableManager. 
    await cube.init(true);
    
    // Solve & verify
    const solution = cube.solve(scrambled, { target: cube.cubeId, goodEnough: 22 });
    if (solution.ok) {
        console.log("solution: ", solution.solution);
        const endState = cube.facecube(solution.solution, scrambled);
        console.log("verified:", endState === cube.cubeId);
    } else {
        console.error("solving failed:", solution.error)
    }
}
await main();