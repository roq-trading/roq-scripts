{% include 'copyright' %}

#pragma once

#include "roq/compat/fmt.hpp"

{% include 'fmt_includes' %}


{% include 'std_includes' %}


{% include 'namespace_begin' %}

struct {{ name }} {
{% for value in values %}
{%- if value.is_string %}
  std::string_view const {{ value.name }};
{%- else %}
{%- if value.is_pod_or_std %}
  {{ value.type }} const {{ value.name }};
{%- else %}
  {{ value.type }} const & {{ value.name }};
{%- endif %}
{%- endif %}
{%- endfor %}

  static {{ name }} create();
};

{% include 'namespace_end' %}

{% include 'format' %}
