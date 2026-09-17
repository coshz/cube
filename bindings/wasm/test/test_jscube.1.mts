import { createCubeAPI, CubeID } from '@coshz/jscube';

const cube = await createCubeAPI();

const fake_cube = "DUUBULDBRBLURRFFLDFLDFBBDFFDURRDRUURDFLLLRBRLBBRFFFLUD";
console.log(cube.solvable(fake_cube));

const scrambled = cube.facecube("U F U' L2 R L' D2 B")

try {
    const solution = cube.solve(scrambled);
    console.log(solution);
} catch (e) {
    console.log(e)
}