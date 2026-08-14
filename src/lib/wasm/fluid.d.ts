// Hand-written types for the Emscripten glue emitted by `wasm/Makefile`.
// The generated fluid.js is minified output and carries no types of its own.

export interface FluidModule {
	/** Allocate an n x n simulation grid. Must be called before anything else. */
	_fluid_init(n: number): void;
	/** Zero the velocity and dye fields, keeping the current grid size. */
	_fluid_reset(): void;
	_fluid_set_params(viscosity: number, diffusion: number, fade: number, vorticity: number): void;
	/**
	 * Inject velocity and dye with a gaussian falloff.
	 * x/y and radius are normalised to [0, 1]; dx/dy are grid units per second.
	 */
	_fluid_splat(
		x: number,
		y: number,
		dx: number,
		dy: number,
		r: number,
		g: number,
		b: number,
		radius: number
	): void;
	/** Advance the solver. dt is clamped to [0.001, 0.05] on the C++ side. */
	_fluid_step(dt: number): void;
	/** Pack the dye field to RGBA8 and return a pointer into HEAPU8. */
	_fluid_render(): number;
	_fluid_size(): number;
	/** Typed as ArrayBuffer (not ArrayBufferLike) so heap views can back an ImageData. */
	HEAPU8: Uint8Array<ArrayBuffer>;
}

declare const createFluidModule: (options?: Record<string, unknown>) => Promise<FluidModule>;
export default createFluidModule;
