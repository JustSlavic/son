#include <son.hpp>
#include <stdio.h>

using namespace jslavic;
son create_scheme_from_son(son& value, std::string* key = nullptr) {
	switch (value.type()) {
	case son::type_t::null:
	case son::type_t::boolean:
	case son::type_t::integer:
	case son::type_t::floating:
	case son::type_t::string:
		return { {"key", key ? *key : son()}, {"type", value.type_name()}, {"values", {}} };
	case son::type_t::object: {
		son result = { {"key", key ? *key : son()}, {"type", value.type_name()} };
		son values;

		for (auto [k, v] : value.pairs()) {
			values.push(create_scheme_from_son(v, &k));
		}

		result.push("values", values);
		return result;
	}
	case son::type_t::array: {
		// Don't do arrays yet
		return { {"key", key ? *key : son()}, {"type", value.type_name()}, {"values", {}} };
	}
	}
}


int main() {
	using namespace jslavic;

	printf("Test parsing son object and creating scheme from example_2.son:\n\n");

	parser p("example_2.son");
	son result = p.parse();

	son scheme = create_scheme_from_son(result);

	printf("object:\n");
	pretty_print(result);
	printf("\n\nscheme:\n");
	pretty_print(scheme);
	printf("\n\n");

	printf("Finish testing.\n");
	return 0;
}


