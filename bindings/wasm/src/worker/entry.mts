import { createCubeCore } from '../core.mjs';
import type { CubeCore } from '../core.mjs';
import { OpMemFsTableManager } from '../TableManager/index.mjs';
import type { WorkerRequest, WorkerResponse } from './protocol.mjs';

const post = (msg: WorkerResponse) => (self as any).postMessage(msg);
let core: CubeCore | null = null;

self.onmessage = async (e: MessageEvent<WorkerRequest>) => {
    const msg = e.data;
    if (msg.type === 'init') {
        try {
            const initModule = () =>
                import('#internal/jscube.web.mjs').then((m) => m.default());
            const tm = new OpMemFsTableManager({
                vfsDir: msg.vfsDir,
                opfsDirname: msg.opfsDirname,
            });
            const c = await createCubeCore(initModule);
            await c.init(tm, msg.useSymmetry ?? false);
            core = c;
            post({ type: 'ready' });
        } catch (err) {
            post({ type: 'init-error', message: toMessage(err) });
        }
        return;
    }
    if (msg.type === 'solve') {
        if (!core) {
            post({ type: 'solve', id: msg.id, result: { ok: false, error: 'not initialized' } });
            return;
        }
        let result;
        try {
            result = core.solve(msg.src, msg.options);
        } catch (err) {
            result = { ok: false, error: toMessage(err) } as const;
        }
        post({ type: 'solve', id: msg.id, result });
    }
};

function toMessage(err: unknown): string {
    return err instanceof Error ? err.message : String(err);
}