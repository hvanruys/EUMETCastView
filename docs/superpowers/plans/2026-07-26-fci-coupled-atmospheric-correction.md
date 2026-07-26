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

## Stage 1a — reflectance scale (unplanned, found during Stage 1)

Stage 1 could not be validated until this was fixed: the solver put the Rayleigh
path reflectance over clear ocean at 0.13 where the measured total was 0.10,
which cannot happen, because a surface only adds to the path term. The model was
right; the data scale was wrong by 1.3892 in every solar band, because
`channel_effective_solar_irradiance` was never read. Fixed.

This invalidates the *absolute* numbers in the spec's earlier limb analysis —
the recovered ocean reflectances there were all 39 % low. The *shape* arguments
survive untouched, since a constant factor cannot create a vza-dependent ramp,
and so does the conclusion that the residual is a surface term.

Where it leaves the clear-ocean profile, with Stage 1 and 1a both in:

| vza | 20–65 | 65–70 | 70–75 | 75–80 |
|---|---|---|---|---|
| recovered vis_04 | 0.000–0.015 | 0.018 | 0.045 | 0.064 |

The disc interior is now fully corrected — what remains there is water-leaving
reflectance, which is the right answer. The limb ramp is untouched, as expected:
it is Stage 3's to remove.

## Stage 3 as built

Not a term bolted onto the correction: the sea surface becomes the **lower
boundary of the radiative transfer**, which is how SeaDAS does it. A flat
Fresnel interface reflects each direction into its mirror and nowhere else, so
as an operator it is exactly **diagonal** — and a diagonal is identical in every
Fourier mode, so it enters the adding equations with no truncation error in the
three-mode framework:

```
R_ocean = R + T_up (I - Rs R)^-1 Rs T_down,    Rs = diag(rF(mu))
```

The water below the interface stays black, so subtracting `R_ocean` leaves the
water-leaving reflectance — the ocean colour one wants to see.

Validated against the residual the black-surface model left behind, with no
fitted parameter anywhere:

| vza | model `R_ocean - R` | measured residual |
|---|---|---|
| 60° | 0.0117 | 0.0085 |
| 70° | 0.0214 | 0.0184 |
| 80° | 0.0372 | 0.0446 |

**Sun glint is not modelled.** A flat surface reflects the direct beam
specularly, and a delta in azimuth cannot live in three Fourier modes — it would
smear an oscillating ghost across the disc. So `T_down` excludes the direct
beam. Glint is a separate analytic Cox–Munk term; skylight, which is smooth and
is what draws the limb ramp, is handled exactly. This also means wind speed does
not enter yet.

### The water test, and what is weak about it

`waterFraction` blends the boundary from the recipe's longest-wavelength solar
band, since water is far darker than land toward the red. Blended, not
thresholded, so a misjudged pixel degrades gradually instead of drawing its own
edge into the image. Thresholds 0.05–0.13 sit in the measured gap: recovered
`vis_06` runs 0.03–0.05 over clear ocean against 0.18–0.32 over land.

The weak case is **dark vegetation seen at high view angle**, which a `vis_06`
test reads as water. Congo is near nadir where the term is only 0.005, so it
does not matter there; Amazon at the west limb is the real exposure, and would
be over-corrected by roughly 0.02 in `vis_04`. Recipes carrying `nir_16` or
`vis_08` separate cleanly and are unaffected.

A GSHHS-rasterised land/sea mask is the robust answer and remains open. Note it
is not strictly better: a geographic mask applies the sea surface underneath
cloud, where it is wrong, and the darkness test does not.

## Stage 4 — pseudo-spherical illumination

The terminator kept coming out wrong in one direction or another because a
plane-parallel model recovers only about a third of the observed radiance past
sza 86: it attenuates the solar beam by `exp(-tau/mu0)`, which diverges at 90 and
shadows the entire column past it, when the real atmosphere is lit through a
curved shell.

Textbook pseudo-spherical: **single scattering along the true spherical path,
multiple scattering left plane-parallel**. Two things make it fit here.

First, work in **TOA units**, `pi*L/E0`, dropping the `1/mu0` of a BRF. The
`mu0` cancels out of the single-scattering term exactly, so the quantity stays
finite and meaningful past the terminator — where a BRF is not merely large but
undefined.

Second, the spherical single-scatter integral

```
Iss(muv) = integral 0..tau of exp(-t*Ch(z(t)) - t/muv) dt,   z(t) = H*ln(tau/t)
```

is one-dimensional in `t` and depends on geometry only through `sza` and `muv`,
so it tabulates: 201 solar zeniths x 32 quadrature nodes per band, built with the
solution. `Ch` is the Chapman air mass; past 90 the integration range is cut at
the altitude below which the ray no longer clears the Earth, which is what makes
the upper atmosphere glow after local sunset.

Multiple scattering is taken from the plane-parallel solution at
`min(sza, MsSzaLimit)` and scaled by `Iss(sza)/Iss(clamped)` — it follows the
light that actually gets in. Below the limit the ratio is 1 and the whole thing
reduces to `mu0 * reflectance()` exactly, which is the test that pins it.

Chapman against `1/cos` at sea level: 0.995 at 60 degrees, 0.963 at 80, 0.65 at
88, and **finite (35.4) at 90 where `1/cos` is infinite**. The resulting TOA path
reflectance runs 1.01x plane-parallel at sza 78, 1.12x at 86, 1.48x at 88, and
where plane-parallel gives 2e-17 at 90 the spherical value is 9.8e-3, decaying
smoothly to 2e-8 by sza 100. That decay *is* the twilight.

`pathReflectanceScale` is gone: the physics now supplies what it approximated.
`twilightFade` stays as a backstop, since nothing is trustworthy within a few
degrees of the terminator.

Cost: 39 ms to build all nine solutions, 99 KB each, and 82 ns per pixel-band —
about 2.5 s of a full-resolution compose across twelve threads.

Known limitation: the *view* path is still plane-parallel. That is the standard
pseudo-spherical compromise and it is much the smaller error, since vza is capped
at 85 by VzaLimit while sza runs to 95.

## Stage 2 as built

Subtracting the path term never left the surface — it left the surface seen
*through* the atmosphere:

```
X = T(mu0)*T(muv) * rho_s / (1 - s*rho_s)
```

Inverting is closed-form, `rho_s = X / (T(mu0)T(muv) + s*X)`, and because
`sunZenithFactor` is `1/mu0_eff` the mu0 that would otherwise appear cancels
against it — so this needed no change to `pathReflectance`, only a step after it.
`T` and `s` were already falling out of the doubling run since Stage 1.

Pinned by a round trip rather than a tolerance: forward-model what the atmosphere
leaves behind for a known `rho_s`, invert, require `rho_s` back. Exact to 1e-4
across all eight bands and the whole geometry range.

The recovered/left-behind amplification at sza 40:

| vza | 0 | 40 | 60 | 70 | 80 |
|---|---|---|---|---|---|
| vis_04 | 1.261 | 1.301 | 1.391 | 1.508 | 1.832 |
| vis_06 | 1.056 | 1.064 | 1.083 | 1.108 | 1.184 |

**This changes the image's colour balance, by design.** The blue band is
amplified 20–55 % more than the red, because that is where the atmosphere is
thick — `T` at nadir is 0.895 for `vis_04` against 0.974 for `vis_06`. Leaving
it out was leaving a blue attenuation in place that deepened toward the limb.
Bright targets saturate rather than run away, via the `s*X` bounce term.

The one stage with no measured validation. Round-tripping proves the algebra
inverts the model; whether the model's transmittance suits real scenes is what
an image will show. Over uniform surface the amplification should *flatten* a
field that dips toward the limb — if instead the limb brightens, the pre-inversion
field was already flat and something upstream is over-correcting.

## Status

- [x] Stage 1 — doubling solver
- [x] Stage 1a — reflectance scale
- [x] Stage 2 — surface coupling
- [x] Stage 3 — sea surface
- [x] Stage 4 — pseudo-spherical illumination

All four stages of the target model are in. What remains open, in rough order of
how much it would buy:

- **Sun glint** — a Cox-Munk direct-beam term, and with it wind speed.
- **A real land/sea mask** — the darkness test misreads dark vegetation at high
  view angle as water.
- **Vector RT** — polarisation, worth a few percent for Rayleigh.
- **Spherical view path** — currently plane-parallel; the smaller half of the
  pseudo-spherical compromise, and bounded by VzaLimit anyway.
