<script lang="ts">
	import { profile } from '$lib/data/profile';
</script>

<header class="relative flex min-h-svh flex-col justify-center px-6">
	<div class="mx-auto w-full max-w-3xl">
		<h1 class="text-4xl font-medium tracking-tight text-fg sm:text-6xl">
			{profile.name}<span class="ml-1 animate-pulse text-accent" aria-hidden="true">_</span>
		</h1>
		<p class="mt-4 text-base text-muted sm:text-lg">{profile.tagline}</p>
		<p class="mt-6 max-w-xl text-sm leading-relaxed text-muted">{profile.blurb}</p>

		<nav class="mt-10 flex flex-wrap gap-x-6 gap-y-3 text-sm">
			<a class="text-fg underline-offset-4 hover:text-accent hover:underline" href="#work">work</a>
			<a class="text-fg underline-offset-4 hover:text-accent hover:underline" href="#projects">
				projects
			</a>
			<a class="text-fg underline-offset-4 hover:text-accent hover:underline" href={profile.github}>
				github
			</a>
			<a
				class="text-fg underline-offset-4 hover:text-accent hover:underline"
				href={profile.linkedin}
			>
				linkedin
			</a>
			<a class="text-fg underline-offset-4 hover:text-accent hover:underline" href={profile.resume}>
				resume
			</a>
		</nav>
	</div>

	<!-- Credits the background: it is the real solver, not a video loop. -->
	<div
		class="absolute inset-x-0 bottom-6 mx-auto w-full max-w-3xl px-6 text-xs text-dim tabular-nums"
	>
		<p>
			<span class="text-accent/70">background:</span> navier–stokes · c++ → wasm
		</p>
		<p class="mt-1 hidden sm:block">move your cursor to disturb the fluid</p>
	</div>
</header>
