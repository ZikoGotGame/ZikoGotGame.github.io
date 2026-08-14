# portfolio

Personal site for Zac Tawfick. SvelteKit + Tailwind, fully prerendered to static files.

The page background is a real Navier–Stokes fluid solver written in C++
(`wasm/fluid.cpp`) and compiled to WebAssembly — a semi-Lagrangian "stable
fluids" scheme with vorticity confinement, running on a 160² grid. Moving the
cursor injects velocity and dye into the field.

## Develop

```sh
npm install
npm run dev
```

## Rebuilding the solver

`src/lib/wasm/fluid.js` is generated **and committed**, so `npm run build`
works on hosts with no C++ toolchain. Rebuild it only after editing
`wasm/fluid.cpp`:

```sh
npm run build:wasm
```

This needs Emscripten. On Arch, `pacman -S emscripten` installs to
`/usr/lib/emscripten` without adding it to `PATH`; the Makefile falls back to
that location automatically. The build emits a single self-contained ES module
(`-sSINGLE_FILE=1` inlines the wasm), so there is no separate `.wasm` to serve.

## Editing content

All copy lives in `src/lib/data/`:

| file            | contents                        |
| --------------- | ------------------------------- |
| `profile.ts`    | name, tagline, links, education |
| `experience.ts` | roles and bullets               |
| `projects.ts`   | project cards                   |

## Checks

```sh
npm run check   # svelte-check, strict
npm run lint    # prettier + eslint
npm run build   # prerender to build/
```
