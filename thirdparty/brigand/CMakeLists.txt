set(BRIGAND_GROUP
      brigand/adapted.hpp
      brigand/algorithms.hpp
      brigand/brigand.hpp
      brigand/functions.hpp
      brigand/sequences.hpp
      brigand/types.hpp
)

set(ADAPTED_GROUP
    brigand/adapted/fusion.hpp
    brigand/adapted/integral_list.hpp
    brigand/adapted/list.hpp
    brigand/adapted/pair.hpp
    brigand/adapted/tuple.hpp
    brigand/adapted/fusion.hpp
    brigand/adapted/variant.hpp
)

set(ALGORITHMS_GROUP
    brigand/algorithms/all.hpp
    brigand/algorithms/any.hpp
    brigand/algorithms/wrap.hpp
    brigand/algorithms/count.hpp
    brigand/algorithms/find.hpp
    brigand/algorithms/flatten.hpp
    brigand/algorithms/fold.hpp
    brigand/algorithms/for_each.hpp
    brigand/algorithms/for_each_args.hpp
    brigand/algorithms/index_of.hpp
    brigand/algorithms/is_set.hpp
    brigand/algorithms/none.hpp
    brigand/algorithms/partition.hpp
    brigand/algorithms/remove.hpp
    brigand/algorithms/replace.hpp
    brigand/algorithms/reverse.hpp
    brigand/algorithms/split.hpp
    brigand/algorithms/split_at.hpp
    brigand/algorithms/sort.hpp
    brigand/algorithms/transform.hpp
    brigand/algorithms/select.hpp
)

set(ALGORITHMS_DETAIL_GROUP
    brigand/algorithms/detail/find.hpp
    brigand/algorithms/detail/fold.hpp
    brigand/algorithms/detail/non_null.hpp
    brigand/algorithms/detail/replace.hpp
)

set(FUNCTIONS_ARITHMETIC_GROUP
    brigand/functions/arithmetic/complement.hpp
    brigand/functions/arithmetic/divides.hpp
    brigand/functions/arithmetic/identity.hpp
    brigand/functions/arithmetic/max.hpp>
    brigand/functions/arithmetic/min.hpp
    brigand/functions/arithmetic/minus.hpp
    brigand/functions/arithmetic/modulo.hpp
    brigand/functions/arithmetic/negate.hpp
    brigand/functions/arithmetic/next.hpp
    brigand/functions/arithmetic/plus.hpp
    brigand/functions/arithmetic/prev.hpp
    brigand/functions/arithmetic/times.hpp
)

set(FUNCTIONS_BITWISE_GROUP
    brigand/functions/bitwise/bitand.hpp
    brigand/functions/bitwise/bitor.hpp
    brigand/functions/bitwise/bitxor.hpp
    brigand/functions/bitwise/shift_left.hpp
    brigand/functions/bitwise/shift_right.hpp
)

set(FUNCTIONS_COMPARISON_GROUP
    brigand/functions/comparison/equal_to.hpp
    brigand/functions/comparison/greater.hpp
    brigand/functions/comparison/greater_equal.hpp
    brigand/functions/comparison/less.hpp
    brigand/functions/comparison/less_equal.hpp
    brigand/functions/comparison/not_equal_to.hpp
)

set(FUNCTIONS_LOGICAL_GROUP
    brigand/functions/logical/and.hpp
    brigand/functions/logical/not.hpp
    brigand/functions/logical/or.hpp
    brigand/functions/logical/xor.hpp
)

set(FUNCTIONS_LAMBDA_GROUP
    brigand/functions/lambda/substitute.hpp
    brigand/functions/lambda/apply.hpp
    brigand/functions/lambda/bind.hpp
    brigand/functions/lambda/lambda.hpp
    brigand/functions/lambda/protect.hpp
    brigand/functions/lambda/quote.hpp
    brigand/functions/lambda/unpack.hpp
)

set(FUNCTIONS_MISC_GROUP
    brigand/functions/misc/always.hpp
    brigand/functions/misc/sizeof.hpp
    brigand/functions/misc/repeat.hpp
)

set(FUNCTIONS_GROUPS
    brigand/functions/arithmetic.hpp
    brigand/functions/bitwise.hpp
    brigand/functions/comparisons.hpp
    brigand/functions/if.hpp
    brigand/functions/eval_if.hpp
    brigand/functions/logical.hpp
    brigand/functions/lambda.hpp
    brigand/functions/misc.hpp
)

set(SEQUENCES_GROUP
    brigand/sequences/append.hpp
    brigand/sequences/at.hpp
    brigand/sequences/back.hpp
    brigand/sequences/front.hpp
    brigand/sequences/last_element.hpp
    brigand/sequences/keys_as_sequence.hpp
    brigand/sequences/list.hpp
    brigand/sequences/make_sequence.hpp
    brigand/sequences/map.hpp
    brigand/sequences/pair.hpp
    brigand/sequences/range.hpp
    brigand/sequences/size.hpp
    brigand/sequences/set.hpp
    brigand/sequences/values_as_sequence.hpp
)

set(TYPES_GROUP
    brigand/types/bool.hpp
    brigand/types/empty_base.hpp
    brigand/types/has_placeholders.hpp
    brigand/types/has_type.hpp
    brigand/types/integer.hpp
    brigand/types/inherit.hpp
    brigand/types/inherit_linearly.hpp
    brigand/types/is_placeholder.hpp
    brigand/types/args.hpp
    brigand/types/no_such_type.hpp
    brigand/types/type.hpp
    brigand/types/real.hpp
    brigand/types/voidp.hpp
)

set(PLACEHOLDER_GROUP brigand/nothing.cpp)

source_group(thirdparty\\brigand FILES ${BRIGAND_GROUP})
source_group(thirdparty\\brigand\\adapted FILES ${ADAPTED_GROUP})
source_group(thirdparty\\brigand\\algorithms FILES ${ALGORITHMS_GROUP})
source_group(thirdparty\\brigand\\algorithms\\detail FILES ${ALGORITHMS_DETAIL_GROUP})
source_group(thirdparty\\brigand\\functions FILES ${FUNCTIONS_GROUPS})
source_group(thirdparty\\brigand\\functions\\arithmetic FILES ${FUNCTIONS_ARITHMETIC_GROUP})
source_group(thirdparty\\brigand\\functions\\bitwise FILES ${FUNCTIONS_BITWISE_GROUP})
source_group(thirdparty\\brigand\\functions\\lambda FILES ${FUNCTIONS_LAMBDA_GROUP})
source_group(thirdparty\\brigand\\functions\\logical FILES ${FUNCTIONS_LOGICAL_GROUP})
source_group(thirdparty\\brigand\\functions\\misc FILES ${FUNCTIONS_MISC_GROUP})
source_group(thirdparty\\brigand\\functions\\comparison FILES ${FUNCTIONS_COMPARISON_GROUP})
source_group(thirdparty\\brigand\\sequences FILES ${SEQUENCES_GROUP})
source_group(thirdparty\\brigand\\types FILES ${TYPES_GROUP})
source_group(thirdparty\\brigand\\placeholder FILES ${PLACEHOLDER_GROUP})

add_library(brigand
    ${BRIGAND_GROUP}
    ${ADAPTED_GROUP}
    ${ALGORITHMS_GROUP}
    ${ALGORITHMS_DETAIL_GROUP}
    ${FUNCTIONS_GROUPS}
    ${SEQUENCES_GROUP}
    ${TYPES_GROUP}
    ${PLACEHOLDER_GROUP})


target_include_directories(brigand
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})