import assert from 'node:assert';
import { test } from 'node:test';
import { createCubeAPI, CubeID } from '@coshz/jscube';

test('cube solver functionality', async () => {
    const cube = await createCubeAPI();

    const fake_cube = "DUUBULDBRBLURRFFLDFLDFBBDFFDURRDRUURDFLLLRBRLBBRFFFLUD";
    assert.strictEqual(cube.solvable(fake_cube), false);

    const scramble = cube.facecube("U F U' L2 R L' D2 B");

    const res = cube.trySolve(scramble);
    if(!res.ok) {
        assert.fail(res.error);
    } else {
        const endState = cube.facecube(res.solution, scramble);
        assert.strictEqual(endState, CubeID);
    }
});