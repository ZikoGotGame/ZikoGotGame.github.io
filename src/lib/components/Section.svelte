<script lang="ts">
	import type { Snippet } from 'svelte';

	interface Props {
		id: string;
		label: string;
		children: Snippet;
	}

	let { id, label, children }: Props = $props();
</script>

<section {id} class="mx-auto w-full max-w-3xl px-6 py-20 sm:py-24">
	<h2 class="mb-10 text-sm tracking-widest text-dim lowercase">
		<span class="text-accent select-none">//</span>
		{label}
	</h2>
	{@render children()}
</section>
