# FCI coupled atmospheric correction (LUT route)

Supersedes the single-scattering path in
`docs/superpowers/specs/2026-07-25-fci-rayleigh-correction-design.md`.

## Why

Two separate deficiencies, established by measurement on a real disc
(2026-07-26 06:20, see the spec's "What is left at the limb" section):

1. **Single scattering under-predicts path reflectance at large air mass.** The
   `(1 − exp)` factor saturates while `1/(μ₀+μ_v)` keeps growing, so the formula
   is bounded ad hoc rather than solved. At τ·m ≈ 2 the error is tens of percent.
2. **The bright ocean limb is not path radiance at all.** Its required
   correction scales with vza and not with air mass — 1.01× at vza 12°/sza 75°
   against 1.27× at vza 72°/sza 40°, the same air mass. Path radiance is
   reciprocal, hence symmetric in μ₀ and μ_v, so no path model of any scattering
   order can produce this. It is skylight reflected by the sea surface.

**A Rayleigh LUT fixes (1) only.** Fixing (2) needs the surface coupling term
and a sea-surface BRDF. Staged accordingly.

## Target model

```
ρ_toa(μ₀,μ_v,φ) = ρ_atm(μ₀,μ_v,φ) + T(μ₀)·T(μ_v)·ρ_s / (1 − s·ρ_s)
```

- `ρ_atm` — atmospheric intrinsic reflectance, black surface, all orders
- `T(μ)`  — total (direct + diffuse) transmittance
- `s`     — spherical albedo of the atmosphere seen from below
- `ρ_s`   — surface reflectance; Lambertian over land, Cox–Munk + whitecaps +
            water-leaving over sea, which is where the vza ramp lives

Inverting for `ρ_s` is the correction.

## Stage 1 — radiative transfer solver and Rayleigh LUT

Replaces `pathReflectance`'s single-scattering formula, the `VzaLimit` clamp and
the `maxPathReflectance` ceiling, all three of which exist only to paper over
single scattering's divergence. Yields `ρ_atm`, `T(μ)` and `s` — every input the
later stages need.

Scalar doubling for a homogeneous conservative Rayleigh layer:

- Gauss–Legendre quadrature on μ ∈ (0,1], N = 32
- Rayleigh phase function Legendre expansion is exactly `1 + c·P₂(cosΘ)`,
  `c = (1−γ)/(2(1+2γ))`, `γ = δ/(2−δ)`, δ = 0.0279 — so the azimuth dependence is
  **exact** with three Fourier modes, `R = R⁰ + 2R¹cos φ + 2R²cos 2φ`. No
  azimuth grid, no azimuth interpolation.
- Initialise at Δτ = τ/2^K with the thin-layer single-scattering limit
  `R_ij = Δτ·P(−μ_i,μ_j)/(4μ_iμ_j)`, which is exactly what the current code
  computes — so Stage 1 reduces to today's behaviour as τ → 0, and the existing
  small-τ test still pins it.
- Double K times: `Q = (I − R**R)⁻¹`, `R' = R + T̃**Q**R**T̃`, `T̃' = T̃**Q**T̃`,
  with `(A**B)_ij = Σ_k A_ik·2μ_kw_k·B_kj` and T̃ including the direct beam.

Cost is ~16 Mflop for all eight bands, so build it at first use behind
`std::call_once` and cache — no generated source file to go stale, and the unit
tests exercise the solver itself rather than a committed table.

Verification, all exact rather than tolerance-tuned:

- **Energy conservation.** Conservative scattering ⇒ plane albedo + total
  transmittance = 1 for every μ₀. This is the check that catches quadrature,
  normalisation and doubling-equation errors.
- **Reciprocity.** `R(μ₀,μ_v,φ) = R(μ_v,μ₀,φ)` — and unlike the current clamped
  code this must now hold at *all* angles, including the limb.
- **Thin limit.** τ → 0 reproduces `τ·P(Θ)/(4μ₀μ_v)`.
- **Bounded.** ρ ≤ 1 everywhere, with no ceiling needed to make it so.
- **Ring regression.** The existing sweep, unchanged.

Known limitation: scalar, so polarisation is neglected. For pure Rayleigh that
is a few percent, against tens of percent for single scattering. Vector would
mean a 4-component Stokes solver; not worth it before Stage 3 exists.

## Stage 2 — coupling term

`T(μ)` and `s` fall out of the same doubling run. Apply the full expression
above with a Lambertian `ρ_s`, i.e. invert for `ρ_s` instead of subtracting
`ρ_atm`. Land becomes correct; ocean still shows the vza ramp because a
Lambertian surface cannot produce one.

Verification: over land the recovered `ρ_s` must be flatter in vza than today's
~1.8×.

## Stage 3 — sea surface

- Cox–Munk wind-roughened Fresnel BRDF, with the sky-glint term — the integral
  of downwelling diffuse radiance reflected into the view direction. This is the
  term that ramps with μ_v and so the one that removes the ring.
- Whitecap and water-leaving reflectance contributions.
- Requires a **land/sea mask** at 5568² and 11136². The app already carries
  GSHHS (`gshhsData`); rasterising it per grid once and caching is the obvious
  route. This is the largest single unknown in the plan and should be settled
  before Stage 3 starts.

Verification: the clear-ocean profile that currently runs 0.011 → 0.066 across
vza 20 → 85 must come back flat, without land or cloud at the same limb getting
darker.

## Status

- [x] Stage 1
- [ ] Stage 2
- [ ] Stage 3
