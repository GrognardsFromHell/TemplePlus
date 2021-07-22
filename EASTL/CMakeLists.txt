set(PROJECT_NAME EASTL)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "include/EABase/config/eacompiler.h"
    "include/EABase/config/eacompilertraits.h"
    "include/EABase/config/eaplatform.h"
    "include/EABase/eabase.h"
    "include/EABase/eahave.h"
    "include/EABase/earesult.h"
    "include/EABase/eastdarg.h"
    "include/EABase/nullptr.h"
    "include/EABase/version.h"
    "include/EASTL/algorithm.h"
    "include/EASTL/allocator.h"
    "include/EASTL/allocator_malloc.h"
    "include/EASTL/array.h"
    "include/EASTL/bitset.h"
    "include/EASTL/bitvector.h"
    "include/EASTL/bonus/call_traits.h"
    "include/EASTL/bonus/compressed_pair.h"
    "include/EASTL/bonus/fixed_string_abstract.h"
    "include/EASTL/bonus/intrusive_sdlist.h"
    "include/EASTL/bonus/intrusive_slist.h"
    "include/EASTL/bonus/list_map.h"
    "include/EASTL/bonus/ring_buffer.h"
    "include/EASTL/bonus/sort_extra.h"
    "include/EASTL/bonus/sparse_matrix.h"
    "include/EASTL/bonus/string_abstract.h"
    "include/EASTL/chrono.h"
    "include/EASTL/core_allocator.h"
    "include/EASTL/core_allocator_adapter.h"
    "include/EASTL/deque.h"
    "include/EASTL/fixed_allocator.h"
    "include/EASTL/fixed_hash_map.h"
    "include/EASTL/fixed_hash_set.h"
    "include/EASTL/fixed_list.h"
    "include/EASTL/fixed_map.h"
    "include/EASTL/fixed_set.h"
    "include/EASTL/fixed_slist.h"
    "include/EASTL/fixed_string.h"
    "include/EASTL/fixed_substring.h"
    "include/EASTL/fixed_vector.h"
    "include/EASTL/functional.h"
    "include/EASTL/hash_map.h"
    "include/EASTL/hash_set.h"
    "include/EASTL/heap.h"
    "include/EASTL/initializer_list.h"
    "include/EASTL/internal/allocator_traits.h"
    "include/EASTL/internal/allocator_traits_fwd_decls.h"
    "include/EASTL/internal/config.h"
    "include/EASTL/internal/copy_help.h"
    "include/EASTL/internal/enable_shared.h"
    "include/EASTL/internal/fill_help.h"
    "include/EASTL/internal/fixed_pool.h"
    "include/EASTL/internal/function.h"
    "include/EASTL/internal/functional_base.h"
    "include/EASTL/internal/generic_iterator.h"
    "include/EASTL/internal/hashtable.h"
    "include/EASTL/internal/integer_sequence.h"
    "include/EASTL/internal/intrusive_hashtable.h"
    "include/EASTL/internal/mem_fn.h"
    "include/EASTL/internal/move_help.h"
    "include/EASTL/internal/pair_fwd_decls.h"
    "include/EASTL/internal/red_black_tree.h"
    "include/EASTL/internal/smart_ptr.h"
    "include/EASTL/internal/thread_support.h"
    "include/EASTL/internal/tuple_fwd_decls.h"
    "include/EASTL/internal/type_compound.h"
    "include/EASTL/internal/type_fundamental.h"
    "include/EASTL/internal/type_pod.h"
    "include/EASTL/internal/type_properties.h"
    "include/EASTL/internal/type_transformations.h"
    "include/EASTL/intrusive_hash_map.h"
    "include/EASTL/intrusive_hash_set.h"
    "include/EASTL/intrusive_list.h"
    "include/EASTL/intrusive_ptr.h"
    "include/EASTL/iterator.h"
    "include/EASTL/linked_array.h"
    "include/EASTL/linked_ptr.h"
    "include/EASTL/list.h"
    "include/EASTL/map.h"
    "include/EASTL/memory.h"
    "include/EASTL/numeric.h"
    "include/EASTL/numeric_limits.h"
    "include/EASTL/priority_queue.h"
    "include/EASTL/queue.h"
    "include/EASTL/random.h"
    "include/EASTL/ratio.h"
    "include/EASTL/safe_ptr.h"
    "include/EASTL/scoped_array.h"
    "include/EASTL/scoped_ptr.h"
    "include/EASTL/set.h"
    "include/EASTL/shared_array.h"
    "include/EASTL/shared_ptr.h"
    "include/EASTL/slist.h"
    "include/EASTL/sort.h"
    "include/EASTL/stack.h"
    "include/EASTL/string.h"
    "include/EASTL/tuple.h"
    "include/EASTL/type_traits.h"
    "include/EASTL/unique_ptr.h"
    "include/EASTL/utility.h"
    "include/EASTL/vector.h"
    "include/EASTL/vector_map.h"
    "include/EASTL/vector_multimap.h"
    "include/EASTL/vector_multiset.h"
    "include/EASTL/vector_set.h"
    "include/EASTL/version.h"
    "include/EASTL/weak_ptr.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "source/allocator_eastl.cpp"
    "source/assert.cpp"
    "source/fixed_pool.cpp"
    "source/hashtable.cpp"
    "source/intrusive_list.cpp"
    "source/numeric_limits.cpp"
    "source/red_black_tree.cpp"
    "source/string.cpp"
    "source/thread_support.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE EASTL)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE     "TRUE"
    INTERPROCEDURAL_OPTIMIZATION_TESTWITHIDA "TRUE"
)
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
    $<$<CONFIG:Debug>:
        MultiThreadedDebug
    >
    $<$<CONFIG:Release>:
        MultiThreaded
    >
    $<$<CONFIG:TestWithIda>:
        MultiThreaded
    >
    $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>,$<CONFIG:TestWithIda>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
)
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:"
        "_DEBUG"
    ">"
    "$<$<CONFIG:Release>:"
        "NDEBUG"
    ">"
    "$<$<CONFIG:TestWithIda>:"
        "NDEBUG"
    ">"
    "WIN32;"
    "_LIB;"
    "UNICODE;"
    "_UNICODE"
)

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /Od
        >
        $<$<CONFIG:Release>:
            /O2;
            /Oi
        >
        $<$<CONFIG:TestWithIda>:
            /O2;
            /Oi
        >
        /std:c++17;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        $<$<CONFIG:TestWithIda>:
            /OPT:REF;
            /OPT:ICF
        >
        /SUBSYSTEM:WINDOWS
    )
endif()

