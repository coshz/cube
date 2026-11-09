import { TableManager } from "../core.mjs";
import NodeFsTableManager from "./NodeFsTableManager.mjs";
import OpMemFsTableManager from "./OpMemFsTableManager.mjs";

class NullTableManager implements TableManager {
    private readonly dir = '/tmp/.null_tables';
    getVirtualPath() { return this.dir; }
    async prepare(module: { FS: any }) { module.FS.mkdirTree(this.dir); }
}

export { NodeFsTableManager, OpMemFsTableManager, NullTableManager };