#!/usr/bin/env python3

"""
Copyright (c) 2017-2023, Hans Erik Thrane

Command-line tool to auto-generate source code files based on JSON spec and templates
"""

import argparse
import json
import re
import os

from jinja2 import Environment, FileSystemLoader


def snake_case(name):
    """convert to snake-case"""
    # https://stackoverflow.com/a/1176023
    name = re.sub("(.)([A-Z][a-z]+)", r"\1_\2", name)
    return re.sub("([a-z0-9])([A-Z])", r"\1_\2", name).lower()


defaults = {
    "std::string": "",
    "std::string_view": "{}",
    "std::chrono::nanoseconds": "{}",
    "std::chrono::microseconds": "{}",
    "std::chrono::milliseconds": "{}",
    "std::chrono::seconds": "{}",
    "std::chrono::days": "{}",
    "bool": "false",
    "char": "'\\0'",
    "int8_t": "{}",
    "uint8_t": "{}",
    "int16_t": "{}",
    "uint16_t": "{}",
    "int32_t": "{}",
    "uint32_t": "{}",
    "int64_t": "{}",
    "uint64_t": "{}",
    "std::int32_t": "{}",
    "std::uint32_t": "{}",
    "std::int64_t": "{}",
    "std::uint64_t": "{}",
    "double": "NaN",
    "std::chrono::year_month_day": "{}",
    "std::chrono::hh_mm_ss<std::chrono::milliseconds>": "{}",
}

unsigned = {
    "uint8_t",
    "uint16_t",
    "uint32_t",
    "uint64_t",
    "std::uint32_t",
    "std::uint64_t",
}

chrono = {
    "std::chrono::nanoseconds",
    "std::chrono::microseconds",
    "std::chrono::milliseconds",
    "std::chrono::seconds",
    "std::chrono::days",
    "roq::days",
    "std::chrono::year_month_day",
    "std::chrono::hh_mm_ss<std::chrono::milliseconds>",
}


def is_variable(type_):
    """test if is a variable"""
    return not type_ is None


def is_array(type_):
    """test if is an array"""
    if type_ is None:
        return False
    return "std::span<" in type_ or "std::vector<" in type_


def is_fix(type_):
    """test if is a roq::fix:: type"""
    if type_ is None:
        return False
    return "roq::fix::" in type_


def remove_const(type_):
    return type_[:-6] if type_[-6:] == " const" else type_


def sub_type(type_):
    """find sub-type"""
    if not type_ is None:
        if "std::span<" in type_:
            return remove_const(type_[10:-1])
        if "roq::fix::" in type_:
            return remove_const(type_[10:-1])
        if "roq::core::fix::" in type_:
            return remove_const(type_[16:-1])
        if "core::fix::" in type_:
            return remove_const(type_[11:-1])
    return type_


def get_default_from_type(type_):
    """get default value from type"""
    if is_array(type_) or is_string_like(type_):
        return ""
    if is_fix(type_):
        # return "{}::UNDEFINED".format(type_)
        return "{}"
    if not is_pod_or_std(type_) and not "core::fix::" in type_ and not "fix::" in type_:
        return ""
    ret = defaults.get(type_)
    return ret if isinstance(ret, str) else "{}"


def is_pod_or_std(type_):
    """test if is plain-old-data or std type"""
    return type_ is None or type_ in defaults


def is_string_like(type_):
    """test if is string-like"""
    if type_ is None:
        return False
    return type_ in ("std::string_view", "std::string")


keywords = {
    "private": "private_",
    "delete": "delete_",
}


def _safe(name):
    """make name safe (avoid restricted keywords)"""
    tmp = keywords.get(name)
    return tmp if tmp else name.replace("-", "_")


def _safe_enum(name):
    """make safe enum string (can't start with a digit)"""
    first_char = name[0]
    return "_" + name if first_char.isdigit() else name.replace("-", "_")


def _new_spec_helper(item):
    """aggregate the specification as a dict"""
    name_ = item["name"]
    raw_name = item.get("raw", name_)
    type_ = item.get("type")
    external = item.get("external", False)
    validator = item.get("validator", "")
    flag_name = item.get("flag_name", name_)
    pub = item.get("public", False)
    name = _safe_enum(snake_case(name_))
    safe_name = _safe(name)
    char = type_ == "char"
    string = is_string_like(type_)
    array = is_array(type_)
    pod_or_std = is_pod_or_std(type_)
    enum = not pod_or_std and not array
    is_float = type_ == "double"
    is_chrono = type_ in chrono
    tag = item.get("tag", -1)
    custom = item.get("custom", False)
    default_raw = item.get("default", get_default_from_type(type_))
    # post processing
    if not (array and "std::vector<" in type_):
        default_raw = "{}".format(default_raw)
    if default_raw == "True":
        default_raw = "true"
    if default_raw == "False":
        default_raw = "false"
    if array and "std::vector<" in type_:
        if len(default_raw) > 0:
            if type(default_raw) is list:
                default_raw = '{}({{"{}"s}})'.format(type_, '"s,"'.join(default_raw))
            else:
                default_raw = '{}({{"{}"s}})'.format(type_, default_raw)
        else:
            default_raw = "{}"
    if string and len(default_raw) == 0:
        pass
        # default_raw = '""s'
    elif not (len(default_raw) == 0 or default_raw == "{}"):
        if is_chrono:
            default_raw = "{{{}}}".format(default_raw)
        elif type_ in unsigned:
            default_raw = "{}".format(default_raw)
        elif string or (type_ is not None and "URI" in type_):
            default_raw = '"{}"s'.format(default_raw)
    description = item.get("description")
    return dict(
        raw_name=raw_name,
        name=safe_name,
        enum_value=name.upper(),
        is_variable=is_variable(type_),
        tag=tag,
        custom=custom,
        type=type_,
        is_required=item.get("required", False),
        is_map=item.get("map", False),
        default_raw=default_raw,
        default="" if len(default_raw) == 0 else " = {}".format(default_raw),
        include=not type_ is None,
        is_pod_or_std=pod_or_std,
        is_string=string,
        is_array=array,
        is_enum=enum,
        is_float=is_float,
        is_chrono=is_chrono,
        format_string="'{}'" if char else '"{}"' if string else "[{}]" if array else "{}",
        format_value=('fmt::join(value.{}, ", "sv)' if array else "value.{}").format(safe_name),
        is_external=external,
        description=description,
        validator=validator,
        has_validator=len(validator) > 0,
        flag_name=flag_name,
        is_public=pub,
    )


def _include_helper(namespaces, variable):
    """sometimes we need a custom include"""
    if variable["is_array"]:
        return namespaces + (snake_case(sub_type(variable["type"])),)
    if not "::" in variable["type"]:
        return namespaces + (snake_case(sub_type(variable["type"])),)
    # XXX HACK
    tmp = tuple(variable["type"].split("::"))
    # return tmp[:-1] + (snake_case(tmp[-1]),)
    return tmp[:-1] + ("common",)


def new_spec(path, raw_namespace, namespaces, name, spec, type_, output_type, prefix):
    """aggregate the spec as dict"""
    filename = os.path.splitext(os.path.basename(path))[0]

    values = [_new_spec_helper(item) for item in spec]
    variables = [value for value in values if value["is_variable"]]

    includes = sorted(
        {
            _include_helper(namespaces, variable)
            for variable in variables
            if (
                variable["is_array"]
                and not is_string_like(sub_type(variable["type"]))
                and not sub_type(variable["type"]) in ("double",)
            )
            or variable["is_enum"]
        }
    )

    return dict(
        raw_namespace=raw_namespace,
        namespaces=namespaces,
        name=name,
        filename=filename,
        variables=variables,
        values=values,
        includes=includes,
        prefix=prefix,
    )


def process(output_type, file_type, path, raw_namespace, namespaces, templates):
    """generate the output file based on a template"""
    with open(path) as file:
        doc = json.load(file)
        type_ = doc["type"]
        name = doc["name"]
        values = doc["values"]
        prefix = doc.get("prefix", "")

        env = Environment(loader=FileSystemLoader(templates))
        template = env.get_template(".".join((type_, file_type)))
        spec = new_spec(path, raw_namespace, namespaces, name, values, type_, output_type, prefix)
        result = template.render(**spec)
        print(result)


def main():
    """main function"""
    parser = argparse.ArgumentParser()
    parser.add_argument("--namespace", type=str, help="namespace")
    parser.add_argument("--output-type", type=str, help="type")
    parser.add_argument("--file-type", type=str, help="output file (h/cpp)")
    parser.add_argument("spec", type=str, help="spec file (.json)")
    args = parser.parse_args()

    raw_namespace = args.namespace

    namespaces = [item.replace("-", "_") for item in tuple(raw_namespace.split("/"))]

    dirname = os.path.dirname(os.path.realpath(__file__))
    templates = [
        os.path.join(dirname, "templates", args.output_type),
        os.path.join(dirname, "templates", "common"),
    ]

    process(args.output_type, args.file_type, args.spec, raw_namespace, namespaces, templates)


if __name__ == "__main__":
    main()
