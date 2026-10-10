
import { createCubeApi } from '@coshz/jscube';

async function main() {
  // 1. Initialize API instance
  const cube = await createCubeApi();

  // 2. Check state validity
  const scrambled = cube.facecube("U F U' L2 R L' D2 B")
  console.log(cube.solvable(scrambled)); // true

  // 3. Solve the cube
  try {
    const solution = cube.solve(scrambled, { target: cube.cubeId, goodEnough: 20 });
    console.log("Solution: ", solution);
    // Verify solution
    const endState = cube.facecube(solution, scrambled);
    console.log("Is solved:", endState === cube.cubeId); // true
  } catch (err) {
    console.error("Solving failed:", (err as Error).message);
    return;
  }
}

await main();