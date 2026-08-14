export interface Project {
	title: string;
	tags: string[];
	blurb: string;
	bullets?: string[];
	image?: string;
	alt?: string;
	href?: string;
}

export const projects: Project[] = [
	{
		title: 'EEVDF Scheduler',
		tags: ['C', 'Linux Kernel', 'Bash Scripting', 'Scheduling'],
		blurb:
			'Modified the Direct Rendering Manager subsystem in the Linux kernel to use the EEVDF scheduling policy. The picture is a Vulkan demo I modified to test different priorities.',
		image: '/projects/eevdf.png',
		alt: 'Vulkan demo for testing GPU job priorities'
	},
	{
		title: 'Fluid Simulator',
		tags: ['C++', 'OpenMP', 'CUDA', 'Linux perf'],
		blurb:
			'A real-time fluid simulation engine built on the Navier–Stokes equations for physically accurate dynamics.',
		image: '/projects/fluid.png',
		alt: 'Fluid simulation with red and green dye'
	},
	{
		title: 'Thrust',
		tags: ['C#', 'Unity Engine', 'Game Dev'],
		blurb: 'Rocket game where you navigate different levels without crashing!',
		image: '/projects/thrust.jpg',
		alt: 'Low-poly 3D scene with a cube character and glowing platforms'
	},
	{
		title: 'Tiny Renderer',
		tags: ['C++', 'Linear Algebra', 'GNU Make'],
		blurb:
			'My own 3D rasterizer with no external dependencies that outputs a .tga image from a .obj file.',
		image: '/projects/tiny.jpg',
		alt: 'Wireframe render of a triangulated head mesh'
	}
];
