{% include 'copyright' %}

#pragma once

#include "roq/compat.hpp"


{% include 'fmt_includes' %}


#include <magic_enum/magic_enum_format.hpp>


{% include 'std_includes' %}


#include "roq/event.hpp"
#include "roq/name.hpp"
#include "roq/trace.hpp"
{%- include 'roq_includes' %}

{% include 'namespace_begin' %}

//! {{ comment }}
struct ROQ_PUBLIC {{ name }} final {
{% include 'variables' %}
};

template <>
inline constexpr std::string_view get_name<{{ name }}>() {
  using namespace std::literals;
  return "{{ filename }}"sv;
}

{% include 'namespace_end' %}

{% include 'format' %}
