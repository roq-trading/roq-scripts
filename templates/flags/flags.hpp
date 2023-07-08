{% include 'autogen' %}

#pragma once

#include <fmt/chrono.h>
#include <fmt/compile.h>
#include <fmt/format.h>

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

{% include 'namespace_begin' %}

struct {{ name }} {
  {{ name }}();

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
};

{% include 'namespace_end' %}

{% include 'format' %}
