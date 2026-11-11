import fs from "node:fs";
import type { TableManager } from "../core.mjs";

class NodeFsTableManager implements TableManager {
    private readonly hostDir: string;
    private readonly vfsDir: string;
    private readonly mounted = new WeakSet<object>();

    constructor(opts: { hostDir: string, vfsDir: string }) {
        this.hostDir = opts.hostDir;
        this.vfsDir = opts.vfsDir;
    }

    getVirtualPath(): string { return this.vfsDir; }

    prepare(module: { FS: any; NODEFS: any }): void {
        if (this.mounted.has(module)) return;
        fs.mkdirSync(this.hostDir, { recursive: true });
        if (!module.FS.analyzePath(this.vfsDir).exists) {
            module.FS.mkdirTree(this.vfsDir);
        }
        module.FS.mount(module.NODEFS, { root: this.hostDir }, this.vfsDir);
        this.mounted.add(module);
    }
}

export default NodeFsTableManager;
