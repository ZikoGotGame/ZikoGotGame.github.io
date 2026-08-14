<script lang="ts">
	import { onMount } from 'svelte';
	import type { FluidModule } from '$lib/wasm/fluid';

	interface Props {
		/** Simulation grid resolution (N x N interior cells). */
		grid?: number;
	}

	let { grid = 160 }: Props = $props();

	let canvas: HTMLCanvasElement;
	let failed = $state(false);

	// The sim is bold behind the hero and recedes to a faint wash further down,
	// so body copy never competes with it.
	let opacity = $state(1);

	onMount(() => {
		const reduceMotion = window.matchMedia('(prefers-reduced-motion: reduce)').matches;

		let raf = 0;
		let disposed = false;
		let mod: FluidModule | null = null;

		// Dye palette, sampled per splat so the trail shifts through teal->blue.
		const hues: [number, number, number][] = [
			[0.06, 0.62, 0.72],
			[0.1, 0.45, 0.8],
			[0.02, 0.72, 0.62]
		];

		// --- pointer state -------------------------------------------------
		let px = 0.5;
		let py = 0.5;
		let hasPointer = false;
		let pending: { x: number; y: number; dx: number; dy: number }[] = [];

		function onPointerMove(e: PointerEvent) {
			// Viewport fraction -> grid fraction. The canvas cover-fits a square
			// grid, so part of it is cropped off-screen; without this the dye
			// lands somewhere other than under the cursor.
			const x = 0.5 + (e.clientX / window.innerWidth - 0.5) * viewW;
			const y = 0.5 + (e.clientY / window.innerHeight - 0.5) * viewH;
			if (hasPointer) {
				pending.push({ x, y, dx: (x - px) * 900, dy: (y - py) * 900 });
			}
			px = x;
			py = y;
			hasPointer = true;
		}

		// --- scroll fade ---------------------------------------------------
		function onScroll() {
			const fade = Math.min(1, window.scrollY / Math.max(1, window.innerHeight * 0.9));
			opacity = 1 - fade * 0.86;
		}

		let visible = !document.hidden;
		function onVisibility() {
			visible = !document.hidden;
			last = performance.now();
		}

		// --- render loop ---------------------------------------------------
		let scratch: HTMLCanvasElement | null = null;
		let scratchCtx: CanvasRenderingContext2D | null = null;
		let ctx: CanvasRenderingContext2D | null = null;
		let last = performance.now();
		// Fraction of the simulation grid actually on screen, per axis.
		let viewW = 1;
		let viewH = 1;
		let warmup = 260;
		let warmupBudget = 3;
		let frame = 0;

		/** Blit the solver's dye buffer to the visible canvas. */
		function paint() {
			if (!mod || !ctx || !scratchCtx || !scratch) return;
			const n = mod._fluid_size();
			const ptr = mod._fluid_render();
			// A view straight into the wasm heap -- no per-frame copy.
			const pixels = new Uint8ClampedArray(mod.HEAPU8.buffer, ptr, n * n * 4);
			scratchCtx.putImageData(new ImageData(pixels, n, n), 0, 0);

			// Cover-fit rather than stretch: the grid is square, and scaling it to a
			// wide viewport smears every vortex into a horizontal streak.
			const scale = Math.max(canvas.width / n, canvas.height / n);
			const w = n * scale;
			const h = n * scale;
			ctx.clearRect(0, 0, canvas.width, canvas.height);
			ctx.drawImage(scratch, (canvas.width - w) / 2, (canvas.height - h) / 2, w, h);
		}

		function resize() {
			if (!canvas) return;
			// The sim output is only `n` px across and gets scaled up anyway, so
			// there is no reason to back the canvas at full device pixel ratio.
			const dpr = Math.min(window.devicePixelRatio || 1, 1.5);
			canvas.width = Math.round(window.innerWidth * dpr * 0.5);
			canvas.height = Math.round(window.innerHeight * dpr * 0.5);

			const scale = Math.max(canvas.width / grid, canvas.height / grid);
			viewW = canvas.width / (grid * scale);
			viewH = canvas.height / (grid * scale);

			// Setting canvas.width clears it. The animated path would redraw on the
			// next frame, but the reduced-motion path has no next frame -- without
			// this the background goes blank on any resize (including a mobile
			// browser collapsing its URL bar).
			paint();
		}

		// Two counter-rotating emitters orbiting the centre. They stay well clear
		// of the walls -- dye driven into a boundary just smears along it instead
		// of rolling, which reads as a smudge rather than a fluid. Tuned so the
		// field reaches a steady density instead of saturating over a long visit.
		function ambient(t: number) {
			if (!mod) return;
			for (let k = 0; k < 2; k++) {
				const dir = k === 0 ? 1 : -1;
				const phase = t * 0.00031 * dir + k * 2.4;
				const c = hues[k % hues.length];
				mod._fluid_splat(
					0.5 + 0.22 * Math.cos(phase),
					0.5 + 0.121 * Math.sin(phase * 1.3),
					-7 * Math.sin(phase) * dir,
					3.85 * Math.cos(phase * 1.3),
					c[0] * 0.6,
					c[1] * 0.6,
					c[2] * 0.6,
					0.04
				);
			}
		}

		function draw(now: number) {
			if (disposed) return;
			raf = requestAnimationFrame(draw);
			if (!mod || !ctx || !scratchCtx || !scratch || !visible) {
				last = now;
				return;
			}

			// Below the fold the sim is barely visible -- halve its framerate
			// rather than burn a full budget on something nobody can see.
			frame++;
			if (opacity < 0.3 && frame % 2 === 1) return;

			const dt = Math.min((now - last) / 1000, 1 / 20);
			last = now;

			// Spin the flow up over the first second or so by running extra steps
			// per frame. Doing this in one synchronous burst at startup would
			// block the main thread for most of a second.
			while (warmup > 0 && warmupBudget-- > 0) {
				ambient(now - warmup * 16.67);
				mod._fluid_step(1 / 60);
				warmup--;
			}
			warmupBudget = 3;

			for (const p of pending) {
				const c = hues[Math.floor(now * 0.0004) % hues.length];
				mod._fluid_splat(p.x, p.y, p.dx, p.dy, c[0], c[1], c[2], 0.03);
			}
			pending = [];
			ambient(now);

			mod._fluid_step(dt);
			paint();
		}

		(async () => {
			try {
				const createFluidModule = (await import('$lib/wasm/fluid.js')).default;
				const m = await createFluidModule();
				if (disposed) return;
				mod = m;

				mod._fluid_init(grid);
				// fade is the per-step dye decay. It sets the steady-state density,
				// and the field has to stay sparse enough to read text over: this
				// settles around 14% coverage, i.e. ink ribbons on black.
				mod._fluid_set_params(0, 0, 0.982, 5);

				scratch = document.createElement('canvas');
				scratch.width = grid;
				scratch.height = grid;
				scratchCtx = scratch.getContext('2d');
				ctx = canvas.getContext('2d');
				if (ctx) ctx.imageSmoothingEnabled = true;

				resize();

				if (reduceMotion) {
					// One still frame, no loop. Kept short: this is the only place
					// the solver runs on the main thread uninterrupted.
					for (let i = 0; i < 90; i++) {
						ambient(i * 16.67);
						mod._fluid_step(1 / 60);
					}
					paint();
					return;
				}

				raf = requestAnimationFrame(draw);
			} catch (err) {
				console.error('fluid: wasm init failed', err);
				failed = true;
			}
		})();

		window.addEventListener('pointermove', onPointerMove, { passive: true });
		window.addEventListener('scroll', onScroll, { passive: true });
		window.addEventListener('resize', resize);
		document.addEventListener('visibilitychange', onVisibility);
		onScroll();

		return () => {
			disposed = true;
			cancelAnimationFrame(raf);
			window.removeEventListener('pointermove', onPointerMove);
			window.removeEventListener('scroll', onScroll);
			window.removeEventListener('resize', resize);
			document.removeEventListener('visibilitychange', onVisibility);
		};
	});
</script>

<div class="pointer-events-none fixed inset-0 -z-10 overflow-hidden bg-ink" aria-hidden="true">
	<canvas
		bind:this={canvas}
		class="h-full w-full transition-opacity duration-300"
		style="opacity: {opacity}"
	></canvas>
	{#if failed}
		<!-- Static stand-in if WebAssembly is unavailable. -->
		<div
			class="absolute inset-0"
			style="background: radial-gradient(60% 50% at 50% 30%, rgba(20,120,140,0.28), transparent 70%)"
		></div>
	{/if}
	<!-- Scrim: guarantees text contrast regardless of what the solver paints. -->
	<div
		class="absolute inset-0"
		style="background: linear-gradient(to bottom, rgba(7,7,8,0.35) 0%, rgba(7,7,8,0.55) 45%, rgba(7,7,8,0.88) 100%)"
	></div>
</div>
