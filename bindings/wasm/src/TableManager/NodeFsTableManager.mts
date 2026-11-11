import fs from "node:fs";
import type { TableManager } from "../core.mjs";

class NodeFsTableManager implements TableManager {
    private readonly hostDir: string;
    private readonly mounted = new WeakSet<object>();
    private readonly VFS_MOUNT = "/tmp/jscube/tables";

    constructor(opts: { hostDir: string }) {
        this.hostDir = opts.hostDir;
    }

    getVirtualPath(): string { return this.VFS_MOUNT; }

    prepare(module: { FS: any; NODEFS: any }): void {
        if (this.mounted.has(module)) return;
        fs.mkdirSync(this.hostDir, { recursive: true });
        if (!module.FS.analyzePath(this.VFS_MOUNT).exists) {
            module.FS.mkdirTree(this.VFS_MOUNT);
        }
        module.FS.mount(module.NODEFS, { root: this.hostDir }, this.VFS_MOUNT);
        this.mounted.add(module);
    }
}

export default NodeFsTableManager;
