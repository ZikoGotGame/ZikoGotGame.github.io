<script lang="ts">
	interface Props {
		tags: string[];
	}

	let { tags }: Props = $props();
</script>

<!-- Keyed by index, not by value: tag strings repeat across projects, and a
	duplicate key is a hard each_key_duplicate error that kills hydration for the
	whole page. These lists are static and never reorder, so index keys are exact. -->
<ul class="flex flex-wrap gap-2">
	{#each tags as tag, i (i)}
		<li class="border border-line px-2 py-0.5 text-xs text-muted">{tag}</li>
	{/each}
</ul>
