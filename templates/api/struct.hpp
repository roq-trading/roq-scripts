{% include 'copyright' %}

#pragma once

#include "roq/compat.hpp"


{% include 'fmt_includes' %}


#include <magic_enum/magic_enum_format.hpp>


{% include 'std_includes' %}


{% include 'roq_includes' %}

{% include 'namespace_begin' %}

//! {{ comment }}
struct ROQ_PUBLIC {{ name }} final {
{% include 'variables' %}
};

{% include 'namespace_end' %}

{% include 'format' %}
