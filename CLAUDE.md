# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Commands

```sh
npm run dev         # dev server -- see "Dev-only assertions" below, this is not optional
npm run build       # prerender to build/
npm run preview     # serve the prerendered build
npm run check       # svelte-check, strict
npm run lint        # prettier --check && eslint
npm run format      # prettier --write
npm run build:wasm  # rebuild the C++ solver (only after editing wasm/fluid.cpp)
```

There is no test suite.

### Dev-only assertions: always verify in `npm run dev`

Svelte compiles out several correctness assertions in production builds. `npm run build && npm run preview`
can pass cleanly on code that throws during hydration in dev.

The one that has already bitten this repo: `validate_each_keys` (`each_key_duplicate`). A duplicate key in a
keyed `{#each}` aborts hydration for the **entire page** — no `onMount` runs anywhere, so the fluid background
silently never starts while the server-rendered HTML still looks perfect. It is easy to misread as a rendering
or CSS bug.

Consequence: **`npm run check` + `npm run lint` + `npm run build` passing is not sufficient evidence that the
page works.** Load it in a browser in dev mode.

### Each-blocks must be index-keyed

`eslint-plugin-svelte`'s `require-each-key` rule forbids unkeyed `{#each}`, so every each-block here uses
`{#each items as item, i (i)}`. Do not key by value (`(tag)`, `(project.title)`): the data files contain
repeated strings, and a duplicate key is the hydration-killing error above. These lists are static and never
reorder, so index keys are exact.

## Architecture

### The C++ → WebAssembly pipeline

`wasm/fluid.cpp` is a real Navier–Stokes solver (semi-Lagrangian "stable fluids": advect → diffuse → project,
plus vorticity confinement) on a 160² grid. It is the page background, and it is the point of the site — the
owner is a systems/graphics engineer, so the background is his own C++ executing in the visitor's browser
rather than a video loop.

Key decisions, all load-bearing:

- **`src/lib/wasm/fluid.js` is generated but committed.** `npm run build` therefore works on hosts with no C++
  toolchain. Rebuild it only when `wasm/fluid.cpp` changes, and commit the result.
- **`-sSINGLE_FILE=1`** inlines the wasm as a binary string inside the `.js`, so there is exactly one artifact:
  no runtime fetch, no Vite asset config, nothing to 404 on a static host.
- **Single-threaded on purpose.** pthreads would force COOP/COEP cross-origin-isolation headers for a grid that
  costs ~2.8 ms/frame on one thread. Do not "optimize" this into a threaded build.
- **`emcc` is not on `PATH`** on Arch (`pacman -S emscripten` installs to `/usr/lib/emscripten`). `wasm/Makefile`
  falls back to that path automatically.
- `src/lib/wasm/fluid.d.ts` is hand-written; the emitted glue is minified and untyped. `HEAPU8` is deliberately
  typed `Uint8Array<ArrayBuffer>` (not `ArrayBufferLike`) so a heap view can back an `ImageData` under `strict`.
- The generated file is excluded from both `.prettierignore` and `eslint.config.js`.

### The JS/wasm boundary

`FluidBackground.svelte` owns the whole lifecycle. Non-obvious constraints encoded there:

- The module is **dynamically imported inside `onMount`** so SSR/prerender never touches it.
- `paint()` builds an `ImageData` over a **view directly into the wasm heap** (`mod.HEAPU8.buffer` at the pointer
  returned by `_fluid_render()`), with no per-frame copy. It blits a 160² scratch canvas onto the display canvas.
- The display canvas **cover-fits** the square grid rather than stretching it, so pointer coordinates must be
  remapped through `viewW`/`viewH` or injected dye lands away from the cursor.
- Startup runs a 260-step warm-up **spread across frames** (3/frame). Doing it in one burst blocks the main
  thread for most of a second.
- Scroll drives canvas opacity (bold in the hero, ~0.14 below it); below the fold the sim drops to half framerate.
- `prefers-reduced-motion` takes a separate path: a short synchronous seed, one still frame, no loop.

### Content

All copy lives in `src/lib/data/` (`profile.ts`, `experience.ts`, `projects.ts`) and is transcribed from
`static/Zac_Tawfick_Resume.pdf`. Edit data, not markup. Every `projects.ts` entry is real and points at a
screenshot in `static/projects/`.

`profile.about` is a `string[]`, rendered one `<p>` per entry. Do not collapse it back into one string with
`\n` separators: HTML folds the newline into a space and the paragraph break silently vanishes.

### Config layout

- **There is no `svelte.config.js`.** SvelteKit options live inline in the `sveltekit({ ... })` call in
  `vite.config.ts`, including the adapter and the compiler option that forces runes mode.
- **There is no `tailwind.config.js`.** Tailwind v4 is configured CSS-first: the `@theme` block in
  `src/routes/layout.css` defines the palette (`--color-ink`, `--color-accent`, …) and the mono font stack.
- The site is fully static: `adapter-static` plus `export const prerender = true` in `src/routes/+page.ts`.
- `svelte/no-navigation-without-resolve` is disabled in `eslint.config.js`: every dynamic `href` here is an
  external profile, a `mailto:`, or a file in `static/`, and internal navigation is all literal hash anchors.

### Deploying: the site assumes it is served from the root

Deployed to GitHub Pages by `.github/workflows/deploy.yml` on every push to `main`: `npm ci` → `npm run build`
→ `upload-pages-artifact` (path `build`) → `deploy-pages`. CI needs no Emscripten because
`src/lib/wasm/fluid.js` is committed.

The site must be served from a domain root. Assets are referenced by absolute path — `/me.jpg` in
`+page.svelte`, `/projects/*` in `projects.ts`, `/Zac_Tawfick_Resume.pdf` in `profile.ts`. That is why the repo
is hosted as the **user site** (`ZikoGotGame/ZikoGotGame.github.io` → `https://zikogotgame.github.io/`) rather
than a project repo: a project repo lands at `/portfolio/` and breaks all of them.

Do not move it back to a subdirectory without also setting `kit.paths.base`, routing those paths through `base`
from `$app/paths`, and re-enabling `svelte/no-navigation-without-resolve` — that rule exists specifically to
catch this, and is only safe to leave off while the site is root-hosted.

`static/.nojekyll` is mandatory (it exists — keep it): Jekyll ignores underscore-prefixed directories and
SvelteKit emits everything into `_app/`, so without it every script and stylesheet 404s.

## Verifying solver changes without a browser

If a browser is unavailable, `wasm/fluid.cpp` can still be verified end-to-end. Both of these have caught real
bugs:

- Compile it natively with `g++` against a stub `emscripten.h` (just `#define EMSCRIPTEN_KEEPALIVE`) and drive
  the `extern "C"` entry points from a small harness.
- Import the committed `src/lib/wasm/fluid.js` in Node, step it, and dump `_fluid_render()`'s RGBA buffer to a
  PPM composited over `#070708` — that is what the page actually shows, and visual problems (dye smearing along
  boundary walls, a field too bright to read text over) are obvious in the image and invisible in the numbers.

Note this verifies the solver only. It does not exercise hydration, layout, or anything else that made the
background fail before — see "Dev-only assertions".

The single tuning knob for background density is `fade` in `FluidBackground.svelte` (currently `0.982`, settling
near 14% dye coverage). It is sensitive: `0.997` produces a near-solid field that text cannot be read over.
