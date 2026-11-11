import type { TableManager } from "../core.mjs";

class OpMemFsTableManager implements TableManager {
    private readonly vfsDir: string;
    private readonly opfsDirname: string;
    private opfsRoot?: FileSystemDirectoryHandle;
    private prepared = false;

    private readonly SEAL_FILE = 'coshz_jscube_tables.sealed';

    constructor(opts: { opfsDirname?: string; vfsDir: string }) {
        this.opfsDirname = opts.opfsDirname ?? 'coshz_jscube_tables';
        this.vfsDir = opts.vfsDir;
    }

    getVirtualPath() { return this.vfsDir; }

    async prepare({ FS }: { FS: any }): Promise<void> {
        if (this.prepared) return;
        FS.mkdirTree(this.vfsDir);
        this.opfsRoot = await this.getOpfsDir();

        // isSealed, a flag to mark tables are complete;
        // if not, clean up tables before building new ones
        let isSealed = false;
        try {
            await this.opfsRoot.getFileHandle(this.SEAL_FILE);
            isSealed = true;
        } catch {}

        if(!isSealed) {
            for await (const [name] of this.opfsRoot.entries()) {
                await this.opfsRoot.removeEntry(name);
            }
        }

        for (const name of await this.listOpfsFiles()) {
            const data = await this.readOpfs(name);
            if (data) FS.writeFile(`${this.vfsDir}/${name}`, data);
        }
        this.prepared = true;
    }

    async afterInit({ FS }: { FS: any }): Promise<void> {
        const root = this.opfsRoot ??= await this.getOpfsDir();
        const memfs = new Set<string>(
            FS.readdir(this.vfsDir).filter((n: string) => n !== '.' && n !== '..')
        );
        const opfs = new Set<string>(await this.listOpfsFiles());

        // remove files that aren't needed in memory
        for (const name of opfs)
            if (!memfs.has(name)) await root.removeEntry(name);

        // persist and release memfs files
        for (const name of memfs) {
            const path = `${this.vfsDir}/${name}`;
            const data = FS.readFile(path);
            await this.writeOpfs(name, data);
            FS.unlink(path);
        }
        // seal the tables
        const sealHandle = await root.getFileHandle(this.SEAL_FILE, { create: true });
        const w = await (sealHandle as any).createWritable();
        try {
            await w.write(new Uint8Array([1]));
        } finally {
            await w.close();
        }
    }

    private async getOpfsDir(): Promise<FileSystemDirectoryHandle> {
        const base = await navigator.storage.getDirectory();
        return base.getDirectoryHandle(this.opfsDirname, { create: true });
    }

    private async listOpfsFiles(): Promise<string[]> {
        const out: string[] = [];
        for await (const [name, h] of this.opfsRoot!.entries())
            if (h.kind === 'file' && name !== this.SEAL_FILE) out.push(name);
        return out;
    }

    private async readOpfs(name: string): Promise<Uint8Array | null> {
        try {
            const fh = await this.opfsRoot!.getFileHandle(name);
            return new Uint8Array(await (await fh.getFile()).arrayBuffer());
        } catch (e) {
            if ((e as DOMException).name === 'NotFoundError') return null;
            throw e;
        }
    }

    private async writeOpfs(name: string, data: Uint8Array): Promise<void> {
        const fh = await this.opfsRoot!.getFileHandle(name, { create: true });
        if ('createSyncAccessHandle' in fh) {
            const s: any = await (fh as any).createSyncAccessHandle();
            try { s.truncate(0); s.write(data, { at: 0 }); s.flush(); }
            finally { s.close(); }
        } else {
            const w = await (fh as any).createWritable();
            try { await w.write(data); } finally { await w.close(); }
        }
    }
}

export default OpMemFsTableManager;
