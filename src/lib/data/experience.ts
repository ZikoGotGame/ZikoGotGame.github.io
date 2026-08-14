export interface Role {
	company: string;
	title: string;
	team?: string;
	location: string;
	period: string;
	bullets: string[];
}

export const experience: Role[] = [
	{
		company: 'Netflix',
		title: 'Software Engineer Intern',
		team: 'JVM Ecosystem',
		location: 'Los Gatos, CA',
		period: 'May 2026 — August 2026',
		bullets: [
			'Embedded Python in JVM ML inference services through GraalPy, removing an external CPython service and cutting p99 latency by 33%, and upstreamed POSIX syscall support and an 18% JIT speedup to GraalPy Community Edition.'
		]
	},
	{
		company: 'Millersville University of Pennsylvania',
		title: 'Linux Kernel Researcher',
		location: 'Millersville, PA',
		period: 'January 2026 — Present',
		bullets: [
			'Implemented EEVDF scheduling in the Linux kernel DRM subsystem unifying its priority-specific runqueues and moving lifetimes onto the kref API. Validated for fairness and latency against custom Vulkan GPU workloads.'
		]
	},
	{
		company: 'Gateway Ticketing Systems',
		title: 'Software Developer Intern',
		location: 'Gilbertsville, PA',
		period: 'May 2025 — August 2025',
		bullets: [
			'Built a C#/.NET desktop application giving operators a central interface for deploying, configuring, and managing Windows Services.'
		]
	},
	{
		company: 'Nature Atlas Herbarium',
		title: 'Full Stack Developer Intern',
		location: 'Millersville, PA',
		period: 'December 2024 — January 2025',
		bullets: [
			'Engineered a multi-part upload pipeline for high-volume observation ingestion, backed by MySQL and Google Maps API services for structured geospatial storage and retrieval.'
		]
	}
];
