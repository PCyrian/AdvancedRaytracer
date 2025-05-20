/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#pragma once

#include <immintrin.h>
#include <cstddef>
#include <array>

namespace Simd {

    constexpr int SIMD_WIDTH = 8;

    struct AABB8 {
        alignas(32) float minX[SIMD_WIDTH];
        alignas(32) float minY[SIMD_WIDTH];
        alignas(32) float minZ[SIMD_WIDTH];
        alignas(32) float maxX[SIMD_WIDTH];
        alignas(32) float maxY[SIMD_WIDTH];
        alignas(32) float maxZ[SIMD_WIDTH];

        // Initializes from 8 boxes stored as arrays of mins/maxs
        AABB8(const float* minsX, const float* minsY, const float* minsZ,
              const float* maxsX, const float* maxsY, const float* maxsZ) {
            for (int i = 0; i < SIMD_WIDTH; ++i) {
                minX[i] = minsX[i]; minY[i] = minsY[i]; minZ[i] = minsZ[i];
                maxX[i] = maxsX[i]; maxY[i] = maxsY[i]; maxZ[i] = maxsZ[i];
            }
        }
    };

    // Carnage-level 8-way AABB intersection with early exits and FMA
    inline __m256i hitFast8(
        const __m256& origX, const __m256& origY, const __m256& origZ,
        const __m256& invX,  const __m256& invY,  const __m256& invZ,
        const __m256i& negX,  const __m256i& negY,  const __m256i& negZ,
        const AABB8& b,
        __m256& tmin, __m256& tmax
    ) {
        // Precompute origin * invDir
        __m256 oix = _mm256_mul_ps(origX, invX);
        __m256 oiy = _mm256_mul_ps(origY, invY);
        __m256 oiz = _mm256_mul_ps(origZ, invZ);

        // X axis slab
        __m256 bx0 = _mm256_blendv_ps(_mm256_load_ps(b.minX), _mm256_load_ps(b.maxX), _mm256_castsi256_ps(negX));
        __m256 bx1 = _mm256_blendv_ps(_mm256_load_ps(b.maxX), _mm256_load_ps(b.minX), _mm256_castsi256_ps(negX));
        __m256 tx0 = _mm256_fmsub_ps(bx0, invX, oix);
        __m256 tx1 = _mm256_fmsub_ps(bx1, invX, oix);
        tmin = _mm256_max_ps(tmin, tx0);
        tmax = _mm256_min_ps(tmax, tx1);
        // early out if nothing remains
        __m256i m = _mm256_castps_si256(_mm256_cmp_ps(tmax, tmin, _CMP_GT_OQ));
        if (_mm256_testz_si256(m, m)) return m;

        // Y axis slab
        __m256 by0 = _mm256_blendv_ps(_mm256_load_ps(b.minY), _mm256_load_ps(b.maxY), _mm256_castsi256_ps(negY));
        __m256 by1 = _mm256_blendv_ps(_mm256_load_ps(b.maxY), _mm256_load_ps(b.minY), _mm256_castsi256_ps(negY));
        __m256 ty0 = _mm256_fmsub_ps(by0, invY, oiy);
        __m256 ty1 = _mm256_fmsub_ps(by1, invY, oiy);
        tmin = _mm256_max_ps(tmin, ty0);
        tmax = _mm256_min_ps(tmax, ty1);
        m = _mm256_castps_si256(_mm256_cmp_ps(tmax, tmin, _CMP_GT_OQ));
        if (_mm256_testz_si256(m, m)) return m;

        // Z axis slab
        __m256 bz0 = _mm256_blendv_ps(_mm256_load_ps(b.minZ), _mm256_load_ps(b.maxZ), _mm256_castsi256_ps(negZ));
        __m256 bz1 = _mm256_blendv_ps(_mm256_load_ps(b.maxZ), _mm256_load_ps(b.minZ), _mm256_castsi256_ps(negZ));
        __m256 tz0 = _mm256_fmsub_ps(bz0, invZ, oiz);
        __m256 tz1 = _mm256_fmsub_ps(bz1, invZ, oiz);
        tmin = _mm256_max_ps(tmin, tz0);
        tmax = _mm256_min_ps(tmax, tz1);

        // Final mask
        __m256 mask = _mm256_cmp_ps(tmax, tmin, _CMP_GT_OQ);
        return _mm256_castps_si256(mask);
    }

} // namespace Simd
