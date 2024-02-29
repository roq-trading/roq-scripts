{% include 'copyright' %}

#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

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
