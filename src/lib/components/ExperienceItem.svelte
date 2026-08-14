<script lang="ts">
	import type { Role } from '$lib/data/experience';

	interface Props {
		role: Role;
	}

	let { role }: Props = $props();
</script>

<article class="border-l border-line pl-5 sm:pl-6">
	<p class="text-xs text-dim">{role.period}</p>
	<h3 class="mt-1 text-base text-fg">
		{role.title}
		<span class="text-dim">@</span>
		<span class="text-accent">{role.company}</span>
	</h3>
	<p class="mt-0.5 text-xs text-muted">
		{#if role.team}{role.team} ·
		{/if}{role.location}
	</p>
	<ul class="mt-3 space-y-2">
		{#each role.bullets as bullet, i (i)}
			<li class="flex gap-2.5 text-sm leading-relaxed text-muted">
				<span class="mt-0.5 shrink-0 text-dim select-none">-</span>
				<span>{bullet}</span>
			</li>
		{/each}
	</ul>
</article>
