{% include 'copyright' %}

{% include 'include' %}

#include <absl/flags/declare.h>
#include <absl/flags/flag.h>

#include <limits>
#include <string>

#include "roq/exceptions.hpp"

#include "roq/utils/charconv/from_chars.hpp"

#include "roq/flags/validators.hpp"

using namespace std::literals;
using namespace std::chrono_literals;  // NOLINT

{% for value in values %}
ABSL_FLAG(  //
{%- if value.has_validator %}
    {{ value.validator }},
{%- else %}
{%- if value.type == 'std::vector<uint16_t>' or value.type == 'std::vector<double>' %}
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
{%- if value.default_raw[-3:-1] == 'ns' %}
   absl::Nanoseconds({{ value.default_raw[1:-3] }}),
{%- else %}
{%- if value.default_raw[-3:-1] == 'ms' %}
   absl::Milliseconds({{ value.default_raw[1:-3] }}),
{%- else %}
{%- if value.default_raw[-2] == 's' %}
   absl::Seconds({{ value.default_raw[1:-2] }}),
{%- else %}
    {{ value.default_raw }},  // XXX maybe extend template to more duration types?
{%- endif %}
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
{% endfor %}

{% include 'namespace_begin' %}

namespace {
struct Helper final {
{% for value in values %}
{%- if value.is_string %}
  static std::string_view {{ value.name }}() {
{%- else %}
{%- if value.is_pod_or_std %}
  static {{ value.type }} {{ value.name }}() {
{%- else %}
  static {{ value.type }} const &{{ value.name }}() {
{%- endif %}
{%- endif %}
{%- if value.type == 'std::chrono::nanoseconds' %}
  static {{ value.type }} const result{absl::ToChronoNanoseconds(absl::GetFlag(FLAGS_{{ prefix }}{{ value.flag_name }}))};
{%- else %}
{%- if value.type == 'std::vector<uint16_t>' or value.type == 'std::vector<double>' %}
  auto parse = []() {
    auto flag = absl::GetFlag(FLAGS_{{ prefix }}{{ value.flag_name }});
    {{ value.type }} result;
    using value_type = {{ value.type }}::value_type;
    for (auto& item : flag) {
      auto value = roq::utils::charconv::from_chars<value_type>(item);
      result.emplace_back(value);
    }
    return result;
  };
  static {{ value.type }} const result{parse()};
{%- else %}
{%- if value.is_string %}
{%- if value.is_required %}
  auto helper = [](){
    auto flag = absl::GetFlag(FLAGS_{{ prefix }}{{ value.flag_name }});
    if (std::empty(flag))
      throw roq::RuntimeError{"--{{ prefix }}{{ value.flag_name }} is required"sv};
    return flag;
  };
  static {{ value.type }} const result{helper()};
{%- else %}
  static {{ value.type }} const result{absl::GetFlag(FLAGS_{{ prefix }}{{ value.flag_name }})};
{%- endif %}
{%- else %}
{%- if value.is_pod_or_std %}
{%- if value.is_required %}
  static const {{ value.type }} result{absl::GetFlag(FLAGS_{{ prefix }}{{ value.flag_name }})};
  if (result == {{ value.type }}{})
    throw roq::RuntimeError{"--{{ prefix }}{{ value.flag_name }} is required"sv};
{%- else %}
  static {{ value.type }} const result{absl::GetFlag(FLAGS_{{ prefix }}{{ value.flag_name }})};
{%- endif %}
{%- else %}
  auto helper = [](){
    auto flag = absl::GetFlag(FLAGS_{{ prefix }}{{ value.flag_name }});
{%- if value.is_required %}
    if (std::empty(flag))
      throw roq::RuntimeError{"--{{ prefix }}{{ value.flag_name }} is required"sv};
{%- else %}
{%- if not 'std::vector<' in value.type %}
    if (flag.required() && std::empty(flag))
      throw roq::RuntimeError{"--{{ prefix }}{{ value.flag_name }} is required"sv};
{%- endif %}
{%- endif %}
    return flag;
  };
  static const {{ value.type }} result{helper()};
{%- endif %}
{%- endif %}
{%- endif %}
{%- endif %}
  return result;
}
{% endfor %}
};
}  // namespace

{{ name }} {{ name }}::create() {
  return {
{%- for value in values %}
    .{{ value.name }} = Helper::{{ value.name}}(),
{%- endfor %}
  };
}

{% include 'namespace_end' %}
