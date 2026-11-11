import { createWorkerSide } from './rpc.mjs';
import { createCubeCore } from '../core.mjs';
import { OpMemFsTableManager } from '../TableManager/index.mjs';
import type { WorkerRequest } from './protocol.mjs';

const handler = createWorkerSide({
    createCore: async () => {
        const initModule = () => import('#internal/jscube.web.mjs').then((m) => m.default());
        return createCubeCore(initModule);
    },
    createTableManager: (msg) =>
        new OpMemFsTableManager({
            opfsDir: msg.persistDir ?? "jscube_tables"
        }),
    post: (msg) => (self as any).postMessage(msg),
});

self.onmessage = (e: MessageEvent<WorkerRequest>) => handler(e.data);
