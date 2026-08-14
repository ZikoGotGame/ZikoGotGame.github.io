<script lang="ts">
	import type { Project } from '$lib/data/projects';
	import TagList from './TagList.svelte';

	interface Props {
		project: Project;
	}

	let { project }: Props = $props();
</script>

<article class="overflow-hidden border border-line bg-surface/60">
	{#if project.image}
		<img
			src={project.image}
			alt={project.alt ?? ''}
			loading="lazy"
			decoding="async"
			class="aspect-video w-full border-b border-line bg-ink object-cover"
		/>
	{/if}
	<div class="space-y-3 p-5">
		<h3 class="text-base text-fg">
			{#if project.href}
				<a class="hover:text-accent" href={project.href}>{project.title}</a>
			{:else}
				{project.title}
			{/if}
		</h3>
		<TagList tags={project.tags} />
		<p class="text-sm leading-relaxed text-muted">{project.blurb}</p>
		{#if project.bullets}
			<ul class="space-y-2 pt-1">
				{#each project.bullets as bullet, i (i)}
					<li class="flex gap-2.5 text-sm leading-relaxed text-muted">
						<span class="mt-0.5 shrink-0 text-dim select-none">-</span>
						<span>{bullet}</span>
					</li>
				{/each}
			</ul>
		{/if}
	</div>
</article>
