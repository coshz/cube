import createCubeModule  from './jscube/jscube.mjs';

const cid = 'UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB';

createCubeModule().then((cube) => {

    const ms = "U F U' L2 R L' D2 B";

    var fc = cube.js_facecube(ms,cid);
    console.log(fc);
    var sol = cube.js_solve(fc,true);

    switch(sol.status_code) {
        case cube.status_code.Ok: 
            console.log("Solution: " + sol.solution); 
            break;
        case cube.status_code.Unsolvable:
            console.warn("Unsolvable");
            break;
        case cube.status_code.Not_found:
            console.warn("No solution found");
            break;
        case cube.status_code.Bad_src:
            console.error("Invalid source");
            break;
        case cube.status_code.Bad_tgt:
            console.error("Invalid target");
            break;
        default:
            console.error("Unknown error");
    }
})

