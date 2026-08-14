<script lang="ts">
	import ExperienceItem from '$lib/components/ExperienceItem.svelte';
	import FluidBackground from '$lib/components/FluidBackground.svelte';
	import Hero from '$lib/components/Hero.svelte';
	import ProjectCard from '$lib/components/ProjectCard.svelte';
	import Section from '$lib/components/Section.svelte';
	import TagList from '$lib/components/TagList.svelte';
	import { education, profile } from '$lib/data/profile';
	import { experience } from '$lib/data/experience';
	import { projects } from '$lib/data/projects';
</script>

<svelte:head>
	<title>{profile.name} — {profile.tagline}</title>
	<meta name="description" content={profile.blurb} />
	<meta property="og:title" content="{profile.name} — {profile.tagline}" />
	<meta property="og:description" content={profile.blurb} />
	<meta property="og:type" content="website" />
	<meta name="theme-color" content="#070708" />
</svelte:head>

<FluidBackground />

<main class="relative">
	<Hero />

	<Section id="about" label="about">
		<div class="flex flex-col gap-8 sm:flex-row sm:items-start">
			<img
				src="/me.jpg"
				alt="Portrait of {profile.name}"
				width="112"
				height="112"
				loading="lazy"
				decoding="async"
				class="size-28 shrink-0 border border-line object-cover"
			/>
			<div class="space-y-4 text-sm leading-relaxed text-muted">
				{#each profile.about as paragraph, i (i)}
					<p>{paragraph}</p>
				{/each}
				<div>
					<p class="text-fg">{education.school}</p>
					<p>{education.degree}</p>
					<p class="text-dim">
						{education.graduation} · GPA {education.gpa} · {profile.location}
					</p>
				</div>
				<div>
					<p class="mb-2 text-dim">coursework</p>
					<TagList tags={education.coursework} />
				</div>
				<div>
					<p class="mb-2 text-dim">competitive programming</p>
					<ul class="space-y-1">
						{#each education.awards as award, i (i)}
							<li>{award}</li>
						{/each}
					</ul>
				</div>
			</div>
		</div>
	</Section>

	<Section id="work" label="work">
		<div class="space-y-12">
			{#each experience as role, i (i)}
				<ExperienceItem {role} />
			{/each}
		</div>
	</Section>

	<Section id="projects" label="projects">
		<div class="grid gap-6 sm:grid-cols-2">
			{#each projects as project, i (i)}
				<ProjectCard {project} />
			{/each}
		</div>
	</Section>

	<Section id="contact" label="contact">
		<ul class="space-y-2 text-sm">
			<li>
				<span class="inline-block w-24 text-dim">email</span>
				<a class="text-fg hover:text-accent" href="mailto:{profile.email}">{profile.email}</a>
			</li>
			<li>
				<span class="inline-block w-24 text-dim">github</span>
				<a class="text-fg hover:text-accent" href={profile.github}>{profile.githubHandle}</a>
			</li>
			<li>
				<span class="inline-block w-24 text-dim">linkedin</span>
				<a class="text-fg hover:text-accent" href={profile.linkedin}>{profile.linkedinHandle}</a>
			</li>
			<li>
				<span class="inline-block w-24 text-dim">resume</span>
				<a class="text-fg hover:text-accent" href={profile.resume} download>pdf</a>
			</li>
		</ul>
	</Section>
</main>
