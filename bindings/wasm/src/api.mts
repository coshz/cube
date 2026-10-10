interface SolveOptions {
    target?: string; 
    goodEnough?: number;
}

type SolveResult =
    | { ok: true; solution: string }
    | { ok: false; error: string }
;

const PermFormat = { Face: 0, Cubie: 1, Cycle: 2 } as const;
type PermFormat = typeof PermFormat[keyof typeof PermFormat];

interface CubeApi {
    readonly cubeId: string;
    solvable: (src: string) => boolean;
    facecube: (maneuver: string, cube?: string) => string;
    permutation: (maneuver: string, fmt?: PermFormat) => string;
    trySolve: (src: string, options?: SolveOptions) => SolveResult; // no exception
    solve: (src: string, options?: SolveOptions) => string; // with exception
}

async function createApiFromModule(
    initModule: () => any | Promise<any>
): Promise<CubeApi> {
    const module_ = await initModule();

    const cubeId: string = module_.cubeId;

    function solvable(src:string):boolean {
        return module_.solvable(src);
    }

    function facecube(maneuver:string, cube: string = cubeId) {
        return module_.facecube(maneuver, cube);
    }
    
    function permutation(ms_or_cube:string, fmt?: PermFormat) {
        return module_.permutation(ms_or_cube, fmt ?? PermFormat.Cycle);
    }

    function trySolve(src:string, options?: SolveOptions): SolveResult {
        const { target = cubeId, goodEnough: goodEnough = 22 } = options ? options : {};
        return module_.solve(src, target, goodEnough);
    }

    function solve(src:string, options?: SolveOptions): string {
        const result = trySolve(src, options);
        if (result.ok) {
            return result.solution;
        } else {
            throw new Error(result.error);
        }
    }

    return {
        cubeId,
        solvable,
        facecube,
        permutation,
        trySolve,
        solve,
    };
}

export { createApiFromModule, SolveResult, SolveOptions, PermFormat };
export type { CubeApi };
