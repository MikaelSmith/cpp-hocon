#include <hocon/config_value.hpp>
#include <internal/config_util.hpp>
#include <hocon/config_object.hpp>
#include <internal/objects/simple_config_object.hpp>
#include <hocon/config_origin.hpp>

using namespace std;

namespace hocon {

    config_value::config_value(config_origin origin) :
        _origin(move(origin)) { }

    string config_value::transform_to_string() const {
        return "";
    }

    config_origin const& config_value::origin() const {
        return _origin;
    }

    resolve_status config_value::get_resolve_status() const {
        return resolve_status::RESOLVED;
    }

    string config_value::render() const {
        return render(config_render_options());
    }

    string config_value::render(config_render_options options) const {
        string result;
        render(result, 0, true, "", options);
        return result;
    }

    void config_value::render(std::string &result, int indent, bool at_root, std::string at_key,
                                         config_render_options options) const {
        if (!at_key.empty()) {
            string rendered_key;
            if (options.get_json()) {
                rendered_key = render_json_string(at_key);
            } else {
                rendered_key = render_string_unquoted_if_possible(at_key);
            }

            result += rendered_key;
            if (options.get_json()) {
                result += options.get_formatted() ? " : " : ":";
            } else {
                // in non-JSON we can omit the color or equals before an object
                if (dynamic_cast<const config_object*>(this)) {
                    if (options.get_formatted()) {
                        result += " ";
                    }
                } else {
                    result += "=";
                }
            }
        }
        render(result, indent, at_root, options);
    }

    void config_value::render(std::string &result, int indent, bool at_root,
                                       config_render_options options) const {
        result += transform_to_string();
    }

    shared_config config_value::at_path(config_origin origin, path raw_path) const {
        path parent = raw_path.parent();
        shared_config result = at_key(origin, *raw_path.last());
        while (!parent.empty()) {
            string key = *parent.last();
            result = result->at_key(origin, key);
            parent = parent.parent();
        }
        return result;
    }

    shared_config config_value::at_key(config_origin origin, std::string const& key) const {
        unordered_map<string, shared_value> map { make_pair(key, shared_from_this()) };
        return simple_config_object(origin, map).to_config();
    }

    shared_config config_value::at_key(std::string const& key) const {
        return at_key(config_origin("at_key(" + key + ")"), key);
    }

    shared_config config_value::at_path(std::string const& path_expression) const {
        auto origin = config_origin("at_path(" + path_expression + ")");
        return at_path(move(origin), path::new_path(path_expression));
    }

}  // namespace hocon
