{% include 'autogen' %}

#pragma once

// only needed if any flags have been declared public
#include <absl/flags/declare.h>

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "roq/io/web/uri.hpp"

// only needed if any flags have been declared public
#include "roq/compat.hpp"

{% for value in values %}
{%- if value.is_public %}
ABSL_DECLARE_FLAG({{ value.validator }}, {{ value.name }}) ROQ_PUBLIC;
{%- endif %}
{%- endfor %}

{% include 'namespace_begin' %}

struct {{ name }} final {
{% for value in values %}
{%- if not value.is_external %}
{%- if value.is_string %}
  static std::string_view {{ prefix }}{{ value.name }}();
{%- else %}
{%- if value.is_pod_or_std %}
  static {{ value.type }} {{ prefix }}{{ value.name }}();
{%- else %}
  static const {{ value.type }} &{{ prefix }}{{ value.name }}();
{%- endif %}
{%- endif %}
{%- endif %}
{%- endfor %}
  // external
{%- for value in values %}
{%- if value.is_external %}
{%- if value.is_string %}
  static std::string_view {{ prefix }}{{ value.name }}();
{%- else %}
{%- if value.is_pod_or_std %}
  static {{ value.type }} {{ prefix }}{{ value.name }}();
{%- else %}
  static const {{ value.type }} &{{ prefix }}{{ value.name }}();
{%- endif %}
{%- endif %}
{%- endif %}
{%- endfor %}
};

struct {{ name }}__flags final {
  {{ name }}__flags();

{% for value in values %}
{%- if not value.is_external %}
{%- if value.is_string %}
  std::string_view const {{ value.name }};
{%- else %}
{%- if value.is_pod_or_std %}
  {{ value.type }} const {{ value.name }};
{%- else %}
  {{ value.type }} const& {{ value.name }};
{%- endif %}
{%- endif %}
{%- endif %}
{%- endfor %}
};

{% include 'namespace_end' %}

{% include 'format2' %}
