/*
 * Copyright (c) 2019-2022 Apple Inc. All rights reserved.
 * Copyright Epic Games, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

/* This isn't really a header. It's a preprocessor for-each loop.
   
   To generate code for each page config kind, just do:
   
       #define PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(name, page_config_value) \
           ... the code you want for pas_segregated_page_config_kind_##name \
               and (page_config_value) ...
       #include "pas_segregated_page_config_kind.def"
       #undef PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND
   
   For example, this can be used to create switch statements as an alternative to adding virtual
   functions or fields to segregated_page_config. Generally, we only use this when we have no
   other alternative (like the unified deallocation log). */

PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(
    null,
    ((pas_segregated_page_config){
        .base = {
            .is_enabled = false,
            .heap_config_ptr = NULL,
            .page_config_ptr = NULL,
        },
        .kind = pas_segregated_page_config_kind_null,

        /* We log NULL using kind_and_role = 0, which works out to null/shared. */
        .shared_logging_mode = pas_segregated_deallocation_size_oblivious_logging_mode,
        .exclusive_logging_mode = pas_segregated_deallocation_no_logging_mode
    }))

PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(pas_utility_small,
                                       PAS_UTILITY_HEAP_CONFIG.small_segregated_config)

#if PAS_ENABLE_THINGY
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(thingy_small_segregated,
                                       THINGY_HEAP_CONFIG.small_segregated_config)
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(thingy_medium_segregated,
                                       THINGY_HEAP_CONFIG.medium_segregated_config)
#endif

#if PAS_ENABLE_ISO
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(iso_small_segregated,
                                       ISO_HEAP_CONFIG.small_segregated_config)
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(iso_medium_segregated,
                                       ISO_HEAP_CONFIG.medium_segregated_config)
#endif

#if PAS_ENABLE_ISO_TEST
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(iso_test_small_segregated,
                                       ISO_TEST_HEAP_CONFIG.small_segregated_config)
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(iso_test_medium_segregated,
                                       ISO_TEST_HEAP_CONFIG.medium_segregated_config)
#endif

#if PAS_ENABLE_MINALIGN32
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(minalign32_small_segregated,
                                       MINALIGN32_HEAP_CONFIG.small_segregated_config)
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(minalign32_medium_segregated,
                                       MINALIGN32_HEAP_CONFIG.medium_segregated_config)
#endif

#if PAS_ENABLE_PAGESIZE64K
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(pagesize64k_small_segregated,
                                       PAGESIZE64K_HEAP_CONFIG.small_segregated_config)
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(pagesize64k_medium_segregated,
                                       PAGESIZE64K_HEAP_CONFIG.medium_segregated_config)
#endif

#if PAS_ENABLE_BMALLOC
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(bmalloc_small_segregated,
                                       BMALLOC_HEAP_CONFIG.small_segregated_config)
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(bmalloc_medium_segregated,
                                       BMALLOC_HEAP_CONFIG.medium_segregated_config)
#endif

#if PAS_ENABLE_HOTBIT
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(hotbit_small_segregated,
                                       HOTBIT_HEAP_CONFIG.small_segregated_config)
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(hotbit_medium_segregated,
                                       HOTBIT_HEAP_CONFIG.medium_segregated_config)
#endif

#if PAS_ENABLE_JIT
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(jit_small_segregated,
                                       JIT_HEAP_CONFIG.small_segregated_config)
#endif

#if PAS_ENABLE_VERSE
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(verse_small_segregated,
                                       VERSE_HEAP_CONFIG.small_segregated_config)
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(verse_medium_segregated,
                                       VERSE_HEAP_CONFIG.medium_segregated_config)
#endif

#if PAS_ENABLE_INLINE_MEDIUM_PAGE_HEADER
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(inline_medium_page_header,
									   INLINE_MEDIUM_PAGE_HEADER_CONFIG.small_segregated_config)
#endif

#if PAS_ENABLE_OUTLINE_MEDIUM_PAGE_HEADER
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(outline_medium_page_header,
									   OUTLINE_MEDIUM_PAGE_HEADER_CONFIG.small_segregated_config)
#endif

#if PAS_ENABLE_INLINE_NON_COMMITTABLE_GRANULES
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(inline_non_committable_granules,
									   INLINE_NON_COMMITTABLE_GRANULES_CONFIG.small_segregated_config)
#endif

#if PAS_ENABLE_OUTLINE_NON_COMMITTABLE_GRANULES
PAS_DEFINE_SEGREGATED_PAGE_CONFIG_KIND(outline_non_committable_granules,
									   OUTLINE_NON_COMMITTABLE_GRANULES_CONFIG.small_segregated_config)
#endif

