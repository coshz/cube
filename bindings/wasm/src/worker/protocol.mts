import { SolveOptions, SolveResult } from "../core.mjs";

type WorkerRequest =
    | { type: 'init'; moduleUrl?: string; persistDir: string; useSymmetry: boolean }
    | { type: 'solve'; id: number; src: string; options?: SolveOptions };

type WorkerResponse =
    | { type: 'ready', isFirstTimeBuild: boolean }
    | { type: 'init-error'; message: string }
    | { type: 'solve'; id: number; result: SolveResult };

export { WorkerRequest, WorkerResponse };
