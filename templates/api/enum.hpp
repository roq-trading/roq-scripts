{% include 'copyright' %}

#pragma once

{% include 'std_includes' %}
#include <cstdint>

{% include 'namespace_begin' %}

//! {{ comment }}
enum class {{ name }} : uint8_t {
  UNDEFINED = 0,
{%- for value in values %}
  {{ value.enum_value }},{{ '  //!< {}'.format(value.comment) if value.comment|length > 0 else '' }}
{%- endfor %}
};

{% include 'namespace_end' %}
