declare module "#internal/jscube.*.mjs" {
    const initModule: () => any | Promise<any>;
    export default initModule;
}