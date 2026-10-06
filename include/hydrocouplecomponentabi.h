/*!
 * \file   hydrocouplecomponentabi.h
 * \author Caleb Buahin
 * \brief  The convention by which a shared library publishes a HydroCouple
 *         component, and the ABI stamp that makes loading it safe.
 *
 * HydroCouple v2 components are plain C++ objects — there is no QObject and
 * therefore no Qt plugin system to lean on, and the SDK deliberately ships no
 * loader (`ModelInitializer` takes a caller-supplied resolver). A host that
 * wants to load components from disk must define the contract itself; this
 * header is that contract, kept free of Qt and of Composer types.
 *
 * \par Provenance
 * Upstreamed 2026-09-30 from HydroCoupleComposer's
 * `include/plugins/componentabi.h` (openswmm program plan §B.2.1, phase D0),
 * so every host and every component — Composer, the openswmm engine, SWMMVis,
 * third-party libraries — includes ONE copy of the contract from the interface
 * package instead of carrying its own. Before this there were three: Composer's,
 * one under `hydrocouplesdk/`, and none here.
 *
 * It was meant to move unchanged, and did, except for one line that could not:
 * `HYDROCOUPLE_COMPONENT_IFACE_VERSION` was hardcoded to "2" while the
 * interfaces had moved to ABI 3 (`releaseState`) and then 4 (the
 * contract-consistency round). Every component built against ABI 3 or 4 with
 * the old header stamped itself `iface=2`, so a host would have accepted an
 * ABI-2 component beside an ABI-4 one — the stamp's one job, defeated. The
 * iface number is now a macro the stamp can stringify AND a `static_assert`
 * against `HydroCouple::HYDROCOUPLE_ABI_VERSION`, so bumping the interface ABI
 * without bumping this line is a compile error in every includer.
 *
 * A component library exports exactly two C functions:
 *
 *   const char          *hydrocouple_component_abi_v1(void);
 *   HydroCouple::IComponentInfo *hydrocouple_component_info_v1(void);
 *
 * `HYDROCOUPLE_DECLARE_COMPONENT(InfoType)` emits both.
 *
 * \par Why an ABI stamp
 * Passing C++ objects across a shared-library boundary is only defined when
 * both sides were built with the same toolchain and standard library. There
 * is no portable way to *recover* from a mismatch once a mismatched C++
 * function has been called, so the mismatch has to be detected before that
 * happens. `hydrocouple_component_abi_v1` is pure C returning a string, which
 * is safe to call across any mismatch; the host compares it against its own
 * `HYDROCOUPLE_COMPONENT_ABI_STAMP` and refuses the library on disagreement.
 * The stamp is therefore the *only* symbol that may be called before the
 * check succeeds.
 *
 * \par Ownership
 * The `IComponentInfo` returned by `hydrocouple_component_info_v1` is owned by
 * the library (a function-local static) and must not be deleted by the host;
 * it stays valid until the library is unloaded. Component *instances* created
 * through `IModelComponentInfo::createComponentInstance()` are owned by the
 * host, and — as a direct consequence — must be destroyed before the library
 * that allocated them is unloaded.
 */

#ifndef HYDROCOUPLE_HYDROCOUPLECOMPONENTABI_H
#define HYDROCOUPLE_HYDROCOUPLECOMPONENTABI_H

#include "hydrocouple.h"

// ── Export/visibility ───────────────────────────────────────────────────────
#if defined(_WIN32)
#  define HYDROCOUPLE_COMPONENT_EXPORT __declspec(dllexport)
#else
#  define HYDROCOUPLE_COMPONENT_EXPORT __attribute__((visibility("default")))
#endif

// ── Stamp construction ──────────────────────────────────────────────────────
#define HYDROCOUPLE_ABI_STRINGIFY_(x) #x
#define HYDROCOUPLE_ABI_STRINGIFY(x)  HYDROCOUPLE_ABI_STRINGIFY_(x)

//! Revision of this loading convention itself (not of the interfaces).
#define HYDROCOUPLE_COMPONENT_ABI_REVISION "1"

/*!
 * \brief Interface ABI this convention targets, as a number the preprocessor
 *        can stringify into the stamp.
 *
 * A macro because the stamp is a string literal assembled by the
 * preprocessor, which cannot read `HydroCouple::HYDROCOUPLE_ABI_VERSION` (a
 * `constexpr int`). Not read from version.h either: that header exports only
 * generically-named macros (PROJECT_VERSION_MAJOR) any other project may also
 * define. The duplication this forces is closed by the static_assert below.
 */
#define HYDROCOUPLE_COMPONENT_IFACE_VERSION_NUMBER 4
#define HYDROCOUPLE_COMPONENT_IFACE_VERSION \
    HYDROCOUPLE_ABI_STRINGIFY(HYDROCOUPLE_COMPONENT_IFACE_VERSION_NUMBER)

// One number, two spellings — so they are made to agree at compile time. This
// line is what the Composer-era header lacked: it said "2" through ABI 3 and 4.
static_assert(HYDROCOUPLE_COMPONENT_IFACE_VERSION_NUMBER ==
                  HydroCouple::HYDROCOUPLE_ABI_VERSION,
              "hydrocouplecomponentabi.h: HYDROCOUPLE_COMPONENT_IFACE_VERSION_NUMBER "
              "disagrees with HydroCouple::HYDROCOUPLE_ABI_VERSION. The interface "
              "ABI was bumped without bumping the component stamp; every component "
              "would stamp the wrong iface and hosts would accept mismatched pairs.");

#if defined(__clang__)
#  define HYDROCOUPLE_ABI_COMPILER "clang-" HYDROCOUPLE_ABI_STRINGIFY(__clang_major__)
#elif defined(__GNUC__)
#  define HYDROCOUPLE_ABI_COMPILER "gcc-" HYDROCOUPLE_ABI_STRINGIFY(__GNUC__)
#elif defined(_MSC_VER)
#  define HYDROCOUPLE_ABI_COMPILER "msvc-" HYDROCOUPLE_ABI_STRINGIFY(_MSC_VER)
#else
#  define HYDROCOUPLE_ABI_COMPILER "unknown"
#endif

#if defined(_LIBCPP_VERSION)
#  define HYDROCOUPLE_ABI_STDLIB "libc++"
#elif defined(__GLIBCXX__)
#  define HYDROCOUPLE_ABI_STDLIB "libstdc++"
#elif defined(_MSVC_STL_VERSION)
#  define HYDROCOUPLE_ABI_STDLIB "msvcstl"
#else
#  define HYDROCOUPLE_ABI_STDLIB "unknown"
#endif

/*!
 * \brief The toolchain fingerprint both sides must agree on.
 *
 * Deliberately coarse: compiler family and major version, standard library,
 * pointer width, and the interface major version. Finer granularity would
 * reject libraries that are in fact compatible; coarser would admit ones that
 * are not.
 */
#define HYDROCOUPLE_COMPONENT_ABI_STAMP                                        \
    "hc-abi/" HYDROCOUPLE_COMPONENT_ABI_REVISION                               \
    ";iface=" HYDROCOUPLE_COMPONENT_IFACE_VERSION                              \
    ";cxx=" HYDROCOUPLE_ABI_COMPILER                                           \
    ";stdlib=" HYDROCOUPLE_ABI_STDLIB                                          \
    ";bits=" HYDROCOUPLE_ABI_STRINGIFY(__SIZEOF_POINTER__)

// ── Entry-point names (kept as string literals for the loader's dlsym) ──────
#define HYDROCOUPLE_COMPONENT_ABI_SYMBOL  "hydrocouple_component_abi_v1"
#define HYDROCOUPLE_COMPONENT_INFO_SYMBOL "hydrocouple_component_info_v1"

/*!
 * \brief The pre-existing, unstamped factory symbol.
 *
 * HydroCouple's Python bindings (`hydrocouple.loader.load`) already load
 * components through an `extern "C"` factory named `CreateComponentInfo`
 * returning `IModelComponentInfo *`, with no ABI stamp. Components written
 * against that convention predate this header, and refusing them would split
 * the component ecosystem in two — Python-loadable versus Composer-loadable.
 *
 * Composer therefore accepts both: the stamped entry points when present, and
 * this legacy factory otherwise. Legacy libraries are loaded on a best-effort
 * basis and reported as unstamped, because there is genuinely no way to
 * verify their toolchain before calling into them.
 */
#define HYDROCOUPLE_COMPONENT_LEGACY_INFO_SYMBOL "CreateComponentInfo"

//! Reported as the stamp of a library that carries no stamp of its own.
#define HYDROCOUPLE_COMPONENT_UNSTAMPED "unstamped(legacy CreateComponentInfo)"

extern "C"
{
  //! Signature of the ABI stamp entry point. Safe to call across a mismatch.
  typedef const char *(*HydroCoupleComponentAbiFn)(void);

  //! Signature of the component-info entry point. Only safe once the stamp matches.
  typedef HydroCouple::IComponentInfo *(*HydroCoupleComponentInfoFn)(void);

  //! Signature of the legacy factory, which returns the narrower model-info type.
  typedef HydroCouple::IModelComponentInfo *(*HydroCoupleLegacyComponentInfoFn)(void);
}

/*!
 * \brief Emits both entry points for a component library.
 * \param InfoType A default-constructible HydroCouple::IComponentInfo subclass.
 *
 * Place once in exactly one translation unit of the component library:
 * \code
 *   HYDROCOUPLE_DECLARE_COMPONENT(MyModelComponentInfo)
 * \endcode
 */
#define HYDROCOUPLE_DECLARE_COMPONENT(InfoType)                                \
  extern "C" HYDROCOUPLE_COMPONENT_EXPORT const char *                         \
  hydrocouple_component_abi_v1(void)                                           \
  {                                                                            \
    return HYDROCOUPLE_COMPONENT_ABI_STAMP;                                    \
  }                                                                            \
                                                                               \
  extern "C" HYDROCOUPLE_COMPONENT_EXPORT HydroCouple::IComponentInfo *        \
  hydrocouple_component_info_v1(void)                                          \
  {                                                                            \
    static InfoType s_info;                                                    \
    return &s_info;                                                            \
  }

#endif // HYDROCOUPLE_HYDROCOUPLECOMPONENTABI_H
