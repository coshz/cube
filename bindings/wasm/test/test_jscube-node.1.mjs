import { createCubeAPI } from "../dist/index.mjs";

const api = await createCubeAPI();
const ms = "U F U' L2 R L' D2 B";
var fc = api.facecube(ms);
console.log(fc);
try {
    var sol = api.solve(fc);
    console.log(sol);
} catch (e) {
    console.error(e);
}