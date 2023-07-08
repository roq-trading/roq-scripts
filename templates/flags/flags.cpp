{% include 'autogen' %}

{% include 'include' %}

#include <absl/flags/declare.h>
#include <absl/flags/flag.h>

#include <string>

#include "roq/api.hpp"

using namespace std::literals;
using namespace std::chrono_literals;  // NOLINT

{% for value in values %}
{%- if not value.is_external %}
ABSL_FLAG(  //
{%- if value.has_validator %}
    {{ value.validator }},
{%- else %}
{%- if value.type == 'std::vector<uint16_t>' %}
    std::vector<std::string>,
{%- else %}
{%- if value.type == 'std::chrono::nanoseconds' %}
    absl::Duration,
{%- else %}
    {{ value.type }},
{%- endif %}
{%- endif %}
{%- endif %}
    {{ prefix }}{{ value.flag_name }},
{%- if value.type == 'std::chrono::nanoseconds' %}
{%- if value.default_raw[-3:-1] == 'ms' %}
   absl::Milliseconds({{ value.default_raw[1:-3] }}),
{%- else %}
{%- if value.default_raw[-2] == 's' %}
   absl::Seconds({{ value.default_raw[1:-2] }}),
{%- else %}
    {{ value.default_raw }},  // XXX maybe extend template to more duration types?
{%- endif %}
{%- endif %}
{%- else %}
{%- if value.is_string or value.is_enum %}
    { {{ value.default_raw }} },
{%- else %}
    {{ value.default_raw }},
{%- endif %}
{%- endif %}
    "{{ value.description }}"s);
{%- endif %}
{% endfor %}

// external

{% for value in values %}
{%- if value.is_external %}
{%- if value.has_validator %}
ABSL_DECLARE_FLAG({{ value.validator }}, {{ value.flag_name }});
{%- else %}
ABSL_DECLARE_FLAG({{ value.type }}, {{ value.flag_name }});
{%- endif %}
{%- endif %}
{%- endfor %}

{% include 'namespace_begin' %}

{% for value in values %}
{%- if value.is_string %}
  std::string_view {{ name }}::{{ prefix }}{{ value.name }}() {
{%- else %}
{%- if value.is_pod_or_std %}
  {{ value.type }} {{ name }}::{{ prefix }}{{ value.name }}() {
{%- else %}
  const {{ value.type }} &{{ name }}::{{ prefix }}{{ value.name }}() {
{%- endif %}
{%- endif %}
{%- if value.type == 'std::chrono::nanoseconds' %}
  static const {{ value.type }} result{absl::ToChronoNanoseconds(absl::GetFlag(FLAGS_{{ prefix }}{{ value.flag_name }}))};
{%- else %}
{%- if value.type == 'std::vector<uint16_t>' %}
  auto parse = []() {
    auto flag = absl::GetFlag(FLAGS_{{ prefix }}{{ value.flag_name }});
    {{ value.type }} result;
    for (auto& item : flag) {
      auto value = core::from_chars<uint16_t>(item);
      result.emplace_back(value);
    }
    return result;
  };
  static const {{ value.type }} result{parse()};
{%- else %}
  static const {{ value.type }} result{absl::GetFlag(FLAGS_{{ prefix }}{{ value.flag_name }})};
{%- endif %}
{%- endif %}
  return result;
}
{% endfor %}

{{ name }}__flags::{{ name }}__flags() :
{%- for value in values %}
{%- if not value.is_external %}
{%- if value.is_string %}
    {{ value.name }}{ {{ name }}::{{ prefix }}{{ value.name}}() }
{%- else %}
    {{ value.name }}{ {{ name }}::{{ prefix }}{{ value.name}}() }
{%- endif %}
{%- endif %}
{%- if not loop.last %}
    ,
{%- endif %}
{%- endfor %}
{}

{% include 'namespace_end' %}
