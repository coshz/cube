export function loadNodeWasm(): Promise<any> {
    return import('#internal/jscube.node.mjs').then((m) => m.default());
}

export function loadWebWasm(): Promise<any> {
    return import('#internal/jscube.web.mjs').then((m) => m.default());
}
