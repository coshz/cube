import { parentPort } from 'node:worker_threads';
import { createWorkerSide } from './rpc.mjs';
import { createCubeCore } from '../core.mjs';
import NodeFsTableManager from '../TableManager/NodeFsTableManager.mjs';

if (!parentPort) throw new Error('Must be run as a worker thread');

const handler = createWorkerSide({
    createCore: async () => {
        const initModule = () => import('#internal/jscube.node.mjs').then((m) => m.default());
        return createCubeCore(initModule);
    },
    createTableManager: (msg) => {
        return new NodeFsTableManager({ hostDir: msg.persistDir });
    },
    post: (msg) => parentPort!.postMessage(msg),
});

parentPort.on('message', handler);
