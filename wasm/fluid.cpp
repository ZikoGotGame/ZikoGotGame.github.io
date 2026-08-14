// Real-time Navier-Stokes fluid solver.
//
// Semi-Lagrangian "stable fluids" (Stam 1999): the projection step keeps the
// velocity field divergence-free, so the sim stays stable at any timestep --
// which matters here because the browser hands us whatever dt it feels like.
//
// Compiled to WebAssembly and driven from FluidBackground.svelte.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#include <emscripten/emscripten.h>

namespace {

// Interior grid is N x N; the arrays carry a one-cell boundary ring on each
// side, so everything is indexed through IX() over an (N+2)^2 buffer.
int N = 0;
int stride = 0;
int cells = 0;

inline int IX(int x, int y) { return x + y * stride; }

struct Field {
  std::vector<float> cur;
  std::vector<float> prev;

  void resize(int n) {
    cur.assign(n, 0.0f);
    prev.assign(n, 0.0f);
  }
  void swap_buffers() { cur.swap(prev); }
};

Field vx, vy;
Field dye[3]; // r, g, b

std::vector<float> pressure;
std::vector<float> divergence;
std::vector<float> curl;
std::vector<uint8_t> framebuffer;

float viscosity = 0.0f;
float diffusion = 0.0f;
float fade = 0.994f;    // per-step dye decay, keeps the field from saturating
float vorticity = 6.0f; // vorticity confinement strength -- puts the curl back
int pressure_iters = 12;

// Boundary conditions. b == 1 mirrors x-velocity on vertical walls, b == 2
// mirrors y-velocity on horizontal walls, b == 0 is free-slip (dye, pressure).
void set_bnd(int b, std::vector<float> &f) {
  for (int i = 1; i <= N; i++) {
    f[IX(0, i)] = (b == 1) ? -f[IX(1, i)] : f[IX(1, i)];
    f[IX(N + 1, i)] = (b == 1) ? -f[IX(N, i)] : f[IX(N, i)];
    f[IX(i, 0)] = (b == 2) ? -f[IX(i, 1)] : f[IX(i, 1)];
    f[IX(i, N + 1)] = (b == 2) ? -f[IX(i, N)] : f[IX(i, N)];
  }
  f[IX(0, 0)] = 0.5f * (f[IX(1, 0)] + f[IX(0, 1)]);
  f[IX(0, N + 1)] = 0.5f * (f[IX(1, N + 1)] + f[IX(0, N)]);
  f[IX(N + 1, 0)] = 0.5f * (f[IX(N, 0)] + f[IX(N + 1, 1)]);
  f[IX(N + 1, N + 1)] = 0.5f * (f[IX(N, N + 1)] + f[IX(N + 1, N)]);
}

// Gauss-Seidel relaxation, shared by the diffusion and pressure solves.
void lin_solve(int b, std::vector<float> &f, const std::vector<float> &f0,
               float a, float c, int iters) {
  const float inv_c = 1.0f / c;
  for (int k = 0; k < iters; k++) {
    for (int y = 1; y <= N; y++) {
      for (int x = 1; x <= N; x++) {
        const int i = IX(x, y);
        f[i] = (f0[i] +
                a * (f[i - 1] + f[i + 1] + f[i - stride] + f[i + stride])) *
               inv_c;
      }
    }
    set_bnd(b, f);
  }
}

void diffuse(int b, std::vector<float> &f, const std::vector<float> &f0,
             float rate, float dt) {
  if (rate <= 0.0f) {
    f = f0;
    set_bnd(b, f);
    return;
  }
  const float a = dt * rate * float(N) * float(N);
  lin_solve(b, f, f0, a, 1.0f + 4.0f * a, 6);
}

// Trace each cell's centre backwards along the velocity field and sample there.
void advect(int b, std::vector<float> &d, const std::vector<float> &d0,
            const std::vector<float> &u, const std::vector<float> &v,
            float dt) {
  const float dt0 = dt * float(N);
  const float lo = 0.5f;
  const float hi = float(N) + 0.5f;

  for (int j = 1; j <= N; j++) {
    for (int i = 1; i <= N; i++) {
      const int idx = IX(i, j);
      float x = float(i) - dt0 * u[idx];
      float y = float(j) - dt0 * v[idx];

      x = std::clamp(x, lo, hi);
      y = std::clamp(y, lo, hi);

      const int i0 = int(x);
      const int j0 = int(y);
      const int i1 = i0 + 1;
      const int j1 = j0 + 1;

      const float s1 = x - float(i0);
      const float s0 = 1.0f - s1;
      const float t1 = y - float(j0);
      const float t0 = 1.0f - t1;

      d[idx] = s0 * (t0 * d0[IX(i0, j0)] + t1 * d0[IX(i0, j1)]) +
               s1 * (t0 * d0[IX(i1, j0)] + t1 * d0[IX(i1, j1)]);
    }
  }
  set_bnd(b, d);
}

// Hodge decomposition: subtract the gradient of the pressure field so what is
// left is mass-conserving (div u == 0).
void project(std::vector<float> &u, std::vector<float> &v) {
  const float h = 1.0f / float(N);

  for (int j = 1; j <= N; j++) {
    for (int i = 1; i <= N; i++) {
      const int idx = IX(i, j);
      divergence[idx] =
          -0.5f * h *
          (u[idx + 1] - u[idx - 1] + v[idx + stride] - v[idx - stride]);
      pressure[idx] = 0.0f;
    }
  }
  set_bnd(0, divergence);
  set_bnd(0, pressure);

  lin_solve(0, pressure, divergence, 1.0f, 4.0f, pressure_iters);

  for (int j = 1; j <= N; j++) {
    for (int i = 1; i <= N; i++) {
      const int idx = IX(i, j);
      u[idx] -= 0.5f * (pressure[idx + 1] - pressure[idx - 1]) / h;
      v[idx] -= 0.5f * (pressure[idx + stride] - pressure[idx - stride]) / h;
    }
  }
  set_bnd(1, u);
  set_bnd(2, v);
}

// Numerical diffusion in advection eats small vortices; this pushes energy back
// into them so the flow keeps its swirl instead of going smooth and dead.
void confine_vorticity(float dt) {
  if (vorticity <= 0.0f) {
    return;
  }
  for (int j = 1; j <= N; j++) {
    for (int i = 1; i <= N; i++) {
      const int idx = IX(i, j);
      curl[idx] = 0.5f * ((vy.cur[idx + 1] - vy.cur[idx - 1]) -
                          (vx.cur[idx + stride] - vx.cur[idx - stride]));
    }
  }
  for (int j = 2; j < N; j++) {
    for (int i = 2; i < N; i++) {
      const int idx = IX(i, j);
      // Gradient of |curl|, pointing toward the vortex centre.
      const float nx =
          0.5f * (std::fabs(curl[idx + 1]) - std::fabs(curl[idx - 1]));
      const float ny = 0.5f * (std::fabs(curl[idx + stride]) -
                               std::fabs(curl[idx - stride]));
      const float len = std::sqrt(nx * nx + ny * ny) + 1e-5f;
      const float scale = vorticity * dt / len;
      vx.cur[idx] += ny * curl[idx] * scale;
      vy.cur[idx] += -nx * curl[idx] * scale;
    }
  }
}

void velocity_step(float dt) {
  confine_vorticity(dt);

  vx.swap_buffers();
  diffuse(1, vx.cur, vx.prev, viscosity, dt);
  vy.swap_buffers();
  diffuse(2, vy.cur, vy.prev, viscosity, dt);
  project(vx.cur, vy.cur);

  vx.swap_buffers();
  vy.swap_buffers();
  advect(1, vx.cur, vx.prev, vx.prev, vy.prev, dt);
  advect(2, vy.cur, vy.prev, vx.prev, vy.prev, dt);
  project(vx.cur, vy.cur);
}

void dye_step(float dt) {
  for (int c = 0; c < 3; c++) {
    dye[c].swap_buffers();
    diffuse(0, dye[c].cur, dye[c].prev, diffusion, dt);
    dye[c].swap_buffers();
    advect(0, dye[c].cur, dye[c].prev, vx.cur, vy.cur, dt);

    for (float &s : dye[c].cur) {
      s *= fade;
    }
  }
}

} // namespace

extern "C" {

EMSCRIPTEN_KEEPALIVE
void fluid_init(int n) {
  N = std::clamp(n, 16, 512);
  stride = N + 2;
  cells = stride * stride;

  vx.resize(cells);
  vy.resize(cells);
  for (int c = 0; c < 3; c++) {
    dye[c].resize(cells);
  }
  pressure.assign(cells, 0.0f);
  divergence.assign(cells, 0.0f);
  curl.assign(cells, 0.0f);
  framebuffer.assign(size_t(N) * size_t(N) * 4, 0);
}

EMSCRIPTEN_KEEPALIVE
void fluid_reset() {
  std::fill(vx.cur.begin(), vx.cur.end(), 0.0f);
  std::fill(vy.cur.begin(), vy.cur.end(), 0.0f);
  for (int c = 0; c < 3; c++) {
    std::fill(dye[c].cur.begin(), dye[c].cur.end(), 0.0f);
  }
}

EMSCRIPTEN_KEEPALIVE
void fluid_set_params(float visc, float diff, float f, float vort) {
  viscosity = visc;
  diffusion = diff;
  fade = std::clamp(f, 0.5f, 1.0f);
  vorticity = std::max(0.0f, vort);
}

// Inject velocity and dye with a gaussian falloff. x/y are normalised [0,1];
// dx/dy are velocity in grid units per second.
EMSCRIPTEN_KEEPALIVE
void fluid_splat(float x, float y, float dx, float dy, float r, float g,
                 float b, float radius) {
  if (N == 0) {
    return;
  }
  const float cx = x * float(N);
  const float cy = y * float(N);
  const float rad = std::max(1.0f, radius * float(N));
  const float inv_r2 = 1.0f / (rad * rad);

  const int x0 = std::max(1, int(cx - rad));
  const int x1 = std::min(N, int(cx + rad) + 1);
  const int y0 = std::max(1, int(cy - rad));
  const int y1 = std::min(N, int(cy + rad) + 1);

  for (int j = y0; j <= y1; j++) {
    for (int i = x0; i <= x1; i++) {
      const float ddx = float(i) - cx;
      const float ddy = float(j) - cy;
      const float d2 = ddx * ddx + ddy * ddy;
      const float w = std::exp(-d2 * inv_r2 * 3.0f);
      if (w < 0.002f) {
        continue;
      }
      const int idx = IX(i, j);
      vx.cur[idx] += dx * w;
      vy.cur[idx] += dy * w;
      dye[0].cur[idx] += r * w;
      dye[1].cur[idx] += g * w;
      dye[2].cur[idx] += b * w;
    }
  }
}

EMSCRIPTEN_KEEPALIVE
void fluid_step(float dt) {
  if (N == 0) {
    return;
  }
  dt = std::clamp(dt, 0.001f, 0.05f);
  velocity_step(dt);
  dye_step(dt);
}

// Pack the dye field into RGBA8 for the canvas. Alpha carries the dye density
// so the page background shows through where the fluid is thin.
EMSCRIPTEN_KEEPALIVE
uint8_t *fluid_render() {
  if (N == 0) {
    return nullptr;
  }
  uint8_t *out = framebuffer.data();
  for (int j = 0; j < N; j++) {
    for (int i = 0; i < N; i++) {
      const int idx = IX(i + 1, j + 1);
      const float r = dye[0].cur[idx];
      const float g = dye[1].cur[idx];
      const float b = dye[2].cur[idx];
      const float a = std::min(1.0f, std::max(r, std::max(g, b)));

      const size_t o = (size_t(j) * size_t(N) + size_t(i)) * 4;
      out[o + 0] = uint8_t(std::min(1.0f, r) * 255.0f);
      out[o + 1] = uint8_t(std::min(1.0f, g) * 255.0f);
      out[o + 2] = uint8_t(std::min(1.0f, b) * 255.0f);
      out[o + 3] = uint8_t(a * 255.0f);
    }
  }
  return out;
}

EMSCRIPTEN_KEEPALIVE
int fluid_size() { return N; }

} // extern "C"
