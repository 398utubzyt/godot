/**************************************************************************/
/*  translation.cpp                                                       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "translation.h"

#include "core/config/project_settings.h"
#include "core/io/resource_loader.h"
#include "core/os/os.h"
#include "core/string/locales.h"

#ifdef TOOLS_ENABLED
#include "main/main.h"
#endif

Dictionary Translation::_get_messages() const {
	Dictionary d;
	for (const KeyValue<StringName, HashMap<StringName, Variant>> &E : translation_map) {
		Dictionary d2;
		for (const KeyValue<StringName, Variant> &E2 : E.value) {
			d2[E2.key] = E2.value;
		}
		d[E.key] = d2;
	}
	return d;
}

Vector<String> Translation::_get_message_list() const {
	Vector<String> msgs;
	msgs.resize(translation_map.size());
	int idx = 0;
	for (const KeyValue<StringName, HashMap<StringName, Variant>> &E : translation_map) {
		for (const KeyValue<StringName, Variant>& E2 : E.value) {
			msgs.set(idx, E2.key);
			idx += 1;
		}
	}

	return msgs;
}

Array Translation::get_translated_message_list() const {
	Array msgs;
	for (const KeyValue<StringName, HashMap<StringName, Variant>> &E : translation_map) {
		for (const KeyValue<StringName, Variant> &E2 : E.value) {
			msgs.push_back(E2.value);
		}
	}

	return msgs;
}

void Translation::_set_messages(const Dictionary &p_messages) {
	List<Variant> keys;
	p_messages.get_key_list(&keys);
	for (const Variant &E : keys) {
		Variant var = p_messages[E];
		if (var.get_type() != Variant::DICTIONARY)
			continue;
		Dictionary contexts = var;
		List<Variant> ckeys;
		contexts.get_key_list(&ckeys);
		for (const Variant &E2 : ckeys) {
			translation_map[E][E2] = contexts[E2];
		}
	}
}

void Translation::set_locale(const String &p_locale) {
	locale = TranslationServer::get_singleton()->standardize_locale(p_locale);

	if (OS::get_singleton()->get_main_loop() && TranslationServer::get_singleton()->get_loaded_locales().has(get_locale())) {
		OS::get_singleton()->get_main_loop()->notification(MainLoop::NOTIFICATION_TRANSLATION_CHANGED);
	}
}


bool Translation::can_add(const StringName &p_key, const Variant &p_text, const StringName &p_context) const {
	if (p_text.get_type() == Variant::OBJECT && p_text.is_null())
		return false;

	bool ret;
	if (GDVIRTUAL_CALL(_can_add, p_key, p_text, p_context, ret)) {
		return ret;
	}

	return true;
}

void Translation::add_message(const StringName &p_key, const Variant &p_text, const StringName &p_context) {
	ERR_FAIL_COND(!can_add(p_key, p_text, p_context));
	translation_map[p_key][p_context] = p_text;
}

void Translation::add_plural_message(const StringName &p_key, const Array &p_texts, const StringName &p_context) {
	ERR_FAIL_COND(!can_add(p_key, p_texts, p_context));
	translation_map[p_key][p_context] = p_texts;
}

Variant Translation::get_message(const StringName &p_key, const StringName &p_context) const {
	Variant ret;
	if (GDVIRTUAL_CALL(_get_message, p_key, p_context, ret)) {
		return ret;
	}

	HashMap<StringName, HashMap<StringName, Variant>>::ConstIterator E = translation_map.find(p_key);
	if (!E) {
		return Variant();
	}

	HashMap<StringName, Variant>::ConstIterator E2 = E->value.find(p_context);
	if (!E2) {
		return Variant();
	}

	return E2->value;
}

Variant Translation::get_plural_message(const StringName &p_key, const StringName &p_key_plural, int p_n, const StringName &p_context) const {
	Variant ret;
	if (GDVIRTUAL_CALL(_get_plural_message, p_key, p_key_plural, p_n, p_context, ret)) {
		return ret;
	}

	ret = get_message(p_key, p_context);
	if (!ret.is_array())
		return ret;

	Array arr = ret;
	if (p_n < 0 || p_n >= arr.size())
		return ret;

	return arr[p_n];
}

void Translation::erase_message(const StringName &p_key, const StringName &p_context) {
	translation_map.erase(p_key);
}

void Translation::get_message_list(List<StringName> *r_messages) const {
	for (const KeyValue<StringName, HashMap<StringName, Variant>> &E : translation_map) {
		const StringName &key = E.key;
		r_messages->push_back(key);
	}
}

int Translation::get_message_count() const {
	return translation_map.size();
}

void Translation::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_locale", "locale"), &Translation::set_locale);
	ClassDB::bind_method(D_METHOD("get_locale"), &Translation::get_locale);
	ClassDB::bind_method(D_METHOD("can_add", "key", "text", "context"), &Translation::add_message, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("add_message", "key", "text", "context"), &Translation::add_message, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("add_plural_message", "key", "texts", "context"), &Translation::add_plural_message, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("get_message", "key", "context"), &Translation::get_message, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("get_plural_message", "key", "key_plural", "n", "context"), &Translation::get_plural_message, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("erase_message", "key", "context"), &Translation::erase_message, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("get_message_list"), &Translation::_get_message_list);
	ClassDB::bind_method(D_METHOD("get_translated_message_list"), &Translation::get_translated_message_list);
	ClassDB::bind_method(D_METHOD("get_message_count"), &Translation::get_message_count);
	ClassDB::bind_method(D_METHOD("_set_messages", "messages"), &Translation::_set_messages);
	ClassDB::bind_method(D_METHOD("_get_messages"), &Translation::_get_messages);

	GDVIRTUAL_BIND(_can_add, "key", "text", "context");
	GDVIRTUAL_BIND(_get_message, "key", "context");
	GDVIRTUAL_BIND(_get_plural_message, "key", "key_plural", "n", "context");

	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "messages", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE | PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY), "_set_messages", "_get_messages");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "locale"), "set_locale", "get_locale");
}

///////////////////////////////////////////////

struct _character_accent_pair {
	const char32_t character;
	const char32_t *accented_character;
};

static _character_accent_pair _character_to_accented[] = {
	{ 'A', U"Å" },
	{ 'B', U"ß" },
	{ 'C', U"Ç" },
	{ 'D', U"Ð" },
	{ 'E', U"É" },
	{ 'F', U"F́" },
	{ 'G', U"Ĝ" },
	{ 'H', U"Ĥ" },
	{ 'I', U"Ĩ" },
	{ 'J', U"Ĵ" },
	{ 'K', U"ĸ" },
	{ 'L', U"Ł" },
	{ 'M', U"Ḿ" },
	{ 'N', U"й" },
	{ 'O', U"Ö" },
	{ 'P', U"Ṕ" },
	{ 'Q', U"Q́" },
	{ 'R', U"Ř" },
	{ 'S', U"Ŝ" },
	{ 'T', U"Ŧ" },
	{ 'U', U"Ũ" },
	{ 'V', U"Ṽ" },
	{ 'W', U"Ŵ" },
	{ 'X', U"X́" },
	{ 'Y', U"Ÿ" },
	{ 'Z', U"Ž" },
	{ 'a', U"á" },
	{ 'b', U"ḅ" },
	{ 'c', U"ć" },
	{ 'd', U"d́" },
	{ 'e', U"é" },
	{ 'f', U"f́" },
	{ 'g', U"ǵ" },
	{ 'h', U"h̀" },
	{ 'i', U"í" },
	{ 'j', U"ǰ" },
	{ 'k', U"ḱ" },
	{ 'l', U"ł" },
	{ 'm', U"m̀" },
	{ 'n', U"ή" },
	{ 'o', U"ô" },
	{ 'p', U"ṕ" },
	{ 'q', U"q́" },
	{ 'r', U"ŕ" },
	{ 's', U"š" },
	{ 't', U"ŧ" },
	{ 'u', U"ü" },
	{ 'v', U"ṽ" },
	{ 'w', U"ŵ" },
	{ 'x', U"x́" },
	{ 'y', U"ý" },
	{ 'z', U"ź" },
};

Vector<TranslationServer::LocaleScriptInfo> TranslationServer::locale_script_info;

HashMap<String, String> TranslationServer::language_map;
HashMap<String, String> TranslationServer::script_map;
HashMap<String, String> TranslationServer::locale_rename_map;
HashMap<String, String> TranslationServer::country_name_map;
HashMap<String, String> TranslationServer::variant_map;
HashMap<String, String> TranslationServer::country_rename_map;

void TranslationServer::init_locale_info() {
	// Init locale info.
	language_map.clear();
	int idx = 0;
	while (language_list[idx][0] != nullptr) {
		language_map[language_list[idx][0]] = String::utf8(language_list[idx][1]);
		idx++;
	}

	// Init locale-script map.
	locale_script_info.clear();
	idx = 0;
	while (locale_scripts[idx][0] != nullptr) {
		LocaleScriptInfo info;
		info.name = locale_scripts[idx][0];
		info.script = locale_scripts[idx][1];
		info.default_country = locale_scripts[idx][2];
		Vector<String> supported_countries = String(locale_scripts[idx][3]).split(",", false);
		for (int i = 0; i < supported_countries.size(); i++) {
			info.supported_countries.insert(supported_countries[i]);
		}
		locale_script_info.push_back(info);
		idx++;
	}

	// Init supported script list.
	script_map.clear();
	idx = 0;
	while (script_list[idx][0] != nullptr) {
		script_map[script_list[idx][1]] = String::utf8(script_list[idx][0]);
		idx++;
	}

	// Init regional variant map.
	variant_map.clear();
	idx = 0;
	while (locale_variants[idx][0] != nullptr) {
		variant_map[locale_variants[idx][0]] = locale_variants[idx][1];
		idx++;
	}

	// Init locale renames.
	locale_rename_map.clear();
	idx = 0;
	while (locale_renames[idx][0] != nullptr) {
		if (!String(locale_renames[idx][1]).is_empty()) {
			locale_rename_map[locale_renames[idx][0]] = locale_renames[idx][1];
		}
		idx++;
	}

	// Init country names.
	country_name_map.clear();
	idx = 0;
	while (country_names[idx][0] != nullptr) {
		country_name_map[String(country_names[idx][0])] = String::utf8(country_names[idx][1]);
		idx++;
	}

	// Init country renames.
	country_rename_map.clear();
	idx = 0;
	while (country_renames[idx][0] != nullptr) {
		if (!String(country_renames[idx][1]).is_empty()) {
			country_rename_map[country_renames[idx][0]] = country_renames[idx][1];
		}
		idx++;
	}
}

String TranslationServer::standardize_locale(const String &p_locale) const {
	return _standardize_locale(p_locale, false);
}

String TranslationServer::_standardize_locale(const String &p_locale, bool p_add_defaults) const {
	// Replaces '-' with '_' for macOS style locales.
	String univ_locale = p_locale.replace("-", "_");

	// Extract locale elements.
	String lang_name, script_name, country_name, variant_name;
	Vector<String> locale_elements = univ_locale.get_slice("@", 0).split("_");
	lang_name = locale_elements[0];
	if (locale_elements.size() >= 2) {
		if (locale_elements[1].length() == 4 && is_ascii_upper_case(locale_elements[1][0]) && is_ascii_lower_case(locale_elements[1][1]) && is_ascii_lower_case(locale_elements[1][2]) && is_ascii_lower_case(locale_elements[1][3])) {
			script_name = locale_elements[1];
		}
		if (locale_elements[1].length() == 2 && is_ascii_upper_case(locale_elements[1][0]) && is_ascii_upper_case(locale_elements[1][1])) {
			country_name = locale_elements[1];
		}
	}
	if (locale_elements.size() >= 3) {
		if (locale_elements[2].length() == 2 && is_ascii_upper_case(locale_elements[2][0]) && is_ascii_upper_case(locale_elements[2][1])) {
			country_name = locale_elements[2];
		} else if (variant_map.has(locale_elements[2].to_lower()) && variant_map[locale_elements[2].to_lower()] == lang_name) {
			variant_name = locale_elements[2].to_lower();
		}
	}
	if (locale_elements.size() >= 4) {
		if (variant_map.has(locale_elements[3].to_lower()) && variant_map[locale_elements[3].to_lower()] == lang_name) {
			variant_name = locale_elements[3].to_lower();
		}
	}

	// Try extract script and variant from the extra part.
	Vector<String> script_extra = univ_locale.get_slice("@", 1).split(";");
	for (int i = 0; i < script_extra.size(); i++) {
		if (script_extra[i].to_lower() == "cyrillic") {
			script_name = "Cyrl";
			break;
		} else if (script_extra[i].to_lower() == "latin") {
			script_name = "Latn";
			break;
		} else if (script_extra[i].to_lower() == "devanagari") {
			script_name = "Deva";
			break;
		} else if (variant_map.has(script_extra[i].to_lower()) && variant_map[script_extra[i].to_lower()] == lang_name) {
			variant_name = script_extra[i].to_lower();
		}
	}

	// Handles known non-ISO language names used e.g. on Windows.
	if (locale_rename_map.has(lang_name)) {
		lang_name = locale_rename_map[lang_name];
	}

	// Handle country renames.
	if (country_rename_map.has(country_name)) {
		country_name = country_rename_map[country_name];
	}

	// Remove unsupported script codes.
	if (!script_map.has(script_name)) {
		script_name = "";
	}

	// Add script code base on language and country codes for some ambiguous cases.
	if (p_add_defaults) {
		if (script_name.is_empty()) {
			for (int i = 0; i < locale_script_info.size(); i++) {
				const LocaleScriptInfo &info = locale_script_info[i];
				if (info.name == lang_name) {
					if (country_name.is_empty() || info.supported_countries.has(country_name)) {
						script_name = info.script;
						break;
					}
				}
			}
		}
		if (!script_name.is_empty() && country_name.is_empty()) {
			// Add conntry code based on script for some ambiguous cases.
			for (int i = 0; i < locale_script_info.size(); i++) {
				const LocaleScriptInfo &info = locale_script_info[i];
				if (info.name == lang_name && info.script == script_name) {
					country_name = info.default_country;
					break;
				}
			}
		}
	}

	// Combine results.
	String out = lang_name;
	if (!script_name.is_empty()) {
		out = out + "_" + script_name;
	}
	if (!country_name.is_empty()) {
		out = out + "_" + country_name;
	}
	if (!variant_name.is_empty()) {
		out = out + "_" + variant_name;
	}
	return out;
}

int TranslationServer::compare_locales(const String &p_locale_a, const String &p_locale_b) const {
	String locale_a = _standardize_locale(p_locale_a, true);
	String locale_b = _standardize_locale(p_locale_b, true);

	if (locale_a == locale_b) {
		// Exact match.
		return 10;
	}

	Vector<String> locale_a_elements = locale_a.split("_");
	Vector<String> locale_b_elements = locale_b.split("_");
	if (locale_a_elements[0] == locale_b_elements[0]) {
		// Matching language, both locales have extra parts.
		// Return number of matching elements.
		int matching_elements = 1;
		for (int i = 1; i < locale_a_elements.size(); i++) {
			for (int j = 1; j < locale_b_elements.size(); j++) {
				if (locale_a_elements[i] == locale_b_elements[j]) {
					matching_elements++;
				}
			}
		}
		return matching_elements;
	} else {
		// No match.
		return 0;
	}
}

String TranslationServer::get_locale_name(const String &p_locale) const {
	String lang_name, script_name, country_name;
	Vector<String> locale_elements = standardize_locale(p_locale).split("_");
	lang_name = locale_elements[0];
	if (locale_elements.size() >= 2) {
		if (locale_elements[1].length() == 4 && is_ascii_upper_case(locale_elements[1][0]) && is_ascii_lower_case(locale_elements[1][1]) && is_ascii_lower_case(locale_elements[1][2]) && is_ascii_lower_case(locale_elements[1][3])) {
			script_name = locale_elements[1];
		}
		if (locale_elements[1].length() == 2 && is_ascii_upper_case(locale_elements[1][0]) && is_ascii_upper_case(locale_elements[1][1])) {
			country_name = locale_elements[1];
		}
	}
	if (locale_elements.size() >= 3) {
		if (locale_elements[2].length() == 2 && is_ascii_upper_case(locale_elements[2][0]) && is_ascii_upper_case(locale_elements[2][1])) {
			country_name = locale_elements[2];
		}
	}

	String name = language_map[lang_name];
	if (!script_name.is_empty()) {
		name = name + " (" + script_map[script_name] + ")";
	}
	if (!country_name.is_empty()) {
		name = name + ", " + country_name_map[country_name];
	}
	return name;
}

Vector<String> TranslationServer::get_all_languages() const {
	Vector<String> languages;

	for (const KeyValue<String, String> &E : language_map) {
		languages.push_back(E.key);
	}

	return languages;
}

String TranslationServer::get_language_name(const String &p_language) const {
	return language_map[p_language];
}

Vector<String> TranslationServer::get_all_scripts() const {
	Vector<String> scripts;

	for (const KeyValue<String, String> &E : script_map) {
		scripts.push_back(E.key);
	}

	return scripts;
}

String TranslationServer::get_script_name(const String &p_script) const {
	return script_map[p_script];
}

Vector<String> TranslationServer::get_all_countries() const {
	Vector<String> countries;

	for (const KeyValue<String, String> &E : country_name_map) {
		countries.push_back(E.key);
	}

	return countries;
}

String TranslationServer::get_country_name(const String &p_country) const {
	return country_name_map[p_country];
}

void TranslationServer::set_locale(const String &p_locale) {
	locale = standardize_locale(p_locale);

	if (OS::get_singleton()->get_main_loop()) {
		OS::get_singleton()->get_main_loop()->notification(MainLoop::NOTIFICATION_TRANSLATION_CHANGED);
	}

	ResourceLoader::reload_translation_remaps();
}

String TranslationServer::get_locale() const {
	return locale;
}

PackedStringArray TranslationServer::get_loaded_locales() const {
	PackedStringArray locales;
	for (const KeyValue<String, Vector<Ref<Translation>>> &E : translations) {
		const String &str = E.key;
		ERR_FAIL_COND_V(str.is_empty(), PackedStringArray());

		locales.push_back(str);
	}

	return locales;
}

bool TranslationServer::has_locale(const String& p_locale) const
{
	return translations.has(p_locale);
}

void TranslationServer::add_translation(const Ref<Translation> &p_translation) {
	String trlocale = p_translation->get_locale();
	if (!has_locale(trlocale)) {
		Vector<Ref<Translation>> vec;
		vec.push_back(p_translation);
		translations.insert(trlocale, vec);
		return;
	}

	translations.get(trlocale).push_back(p_translation);
}

void TranslationServer::remove_translation(const Ref<Translation> &p_translation) {
	String trlocale = p_translation->get_locale();
	ERR_FAIL_COND(!has_locale(trlocale));

	Vector<Ref<Translation>> vec = translations.get(trlocale);
	int index = vec.find(p_translation);
	ERR_FAIL_COND(index < 0);

	vec.remove_at(index);
	if (vec.is_empty())
		translations.erase(trlocale);
}

Array TranslationServer::get_translation_objects(const String &p_locale) const {
	Array res;
	int best_score = 0;

	for (const KeyValue<String, Vector<Ref<Translation>>> &E : translations) {
		String l = E.key;

		int score = compare_locales(p_locale, l);
		if (score > 0 && score >= best_score) {
			const Vector<Ref<Translation>> &t = E.value;
			res.clear();
			for (const Ref<Translation> &r : t) {
				res.push_back(r);
			}
			best_score = score;
			if (score == 10) {
				break; // Exact match, skip the rest.
			}
		}
	}
	return res;
}
int TranslationServer::get_translation_object_count(const String &p_locale) const {
	int res = 0;
	int best_score = 0;

	for (const KeyValue<String, Vector<Ref<Translation>>> &E : translations) {
		String l = E.key;

		int score = compare_locales(p_locale, l);
		if (score > 0 && score >= best_score) {
			const Vector<Ref<Translation>> &t = E.value;
			res = t.size();
			best_score = score;
			if (score == 10) {
				break; // Exact match, skip the rest.
			}
		}
	}

	return res;
}

Ref<Translation> TranslationServer::get_translation_object_at(const String &p_locale, int p_index) const {
	Ref<Translation> res;
	int best_score = 0;

	if (p_index < 0)
		return res;

	for (const KeyValue<String, Vector<Ref<Translation>>> &E : translations) {
		String l = E.key;

		int score = compare_locales(p_locale, l);
		if (score > 0 && score >= best_score) {
			const Vector<Ref<Translation>> &t = E.value;
			if (p_index >= t.size())
				continue;
			res = t[p_index];
			best_score = score;
			if (score == 10) {
				break; // Exact match, skip the rest.
			}
		}
	}
	return res;
}

void TranslationServer::clear() {
	translations.clear();
}

Variant TranslationServer::translate(const StringName &p_message, const StringName &p_context) const {
	// Match given message against the translation catalog for the project locale.

	if (!enabled) {
#ifdef TOOLS_ENABLED
		if (!Engine::get_singleton()->is_editor_hint()) {
			WARN_PRINT("Translation Server is not enabled.");
		}
#endif

		return p_message;
	}

	Variant res = _get_message_from_translations(p_message, p_context, locale, false);

	if (!res && fallback.length() >= 2) {
		res = _get_message_from_translations(p_message, p_context, fallback, false);
	}

	if (!res) {
		return pseudolocalization_enabled ? pseudolocalize(p_message) : p_message;
	}

	// Not ternary because pseudolocalize returns StringName and ruins the return type.
	// TODO: Do something about pseudolocalize().
	if (pseudolocalization_enabled && res.get_type() == Variant::Type::STRING_NAME)
		return pseudolocalize(res);
	return res;
}

Variant TranslationServer::translate_plural(const StringName &p_message, const StringName &p_message_plural, int p_n, const StringName &p_context) const {
	if (!enabled) {
#ifdef TOOLS_ENABLED
		if (!Engine::get_singleton()->is_editor_hint()) {
			WARN_PRINT("Translation Server is not enabled.");
		}
#endif
		if (p_n == 1) {
			return p_message;
		}
		return p_message_plural;
	}

	Variant res = _get_message_from_translations(p_message, p_context, locale, true, p_message_plural, p_n);

	if (!res && fallback.length() >= 2) {
		res = _get_message_from_translations(p_message, p_context, fallback, true, p_message_plural, p_n);
	}

	if (!res) {
		if (p_n == 1) {
			return p_message;
		}
		return p_message_plural;
	}

	return res;
}

Variant TranslationServer::_get_message_from_translations(const StringName &p_message, const StringName &p_context, const String &p_locale, bool plural, const String &p_message_plural, int p_n) const {
	Variant res;
	int best_score = 0;

	for (const KeyValue<String, Vector<Ref<Translation>>> &E : translations) {
		const String &l = E.key;

		int score = compare_locales(p_locale, l);
		if (score > 0 && score >= best_score) {
			const Vector<Ref<Translation>> &ta = E.value;
			for (const Ref<Translation> &E2 : ta) {
				const Ref<Translation> &t = E2;
				ERR_FAIL_COND_V(t.is_null(), p_message);
				Variant r;
				if (!plural) {
					r = t->get_message(p_message, p_context);
				} else {
					r = t->get_plural_message(p_message, p_message_plural, p_n, p_context);
				}
				if (!r) {
					continue;
				}
				res = r;
			}
			best_score = score;
			if (score == 10) {
				break; // Exact match, skip the rest.
			}
		}
	}

	return res;
}

TranslationServer *TranslationServer::singleton = nullptr;

bool TranslationServer::_load_translations(const String &p_from) {
	if (ProjectSettings::get_singleton()->has_setting(p_from)) {
		const Vector<String> &translation_names = GLOBAL_GET(p_from);

		int tcount = translation_names.size();

		if (tcount) {
			const String *r = translation_names.ptr();

			for (int i = 0; i < tcount; i++) {
				Ref<Translation> tr = ResourceLoader::load(r[i]);
				if (tr.is_valid()) {
					add_translation(tr);
				}
			}
		}
		return true;
	}

	return false;
}

void TranslationServer::setup() {
	String test = GLOBAL_DEF("internationalization/locale/test", "");
	test = test.strip_edges();
	if (!test.is_empty()) {
		set_locale(test);
	} else {
		set_locale(OS::get_singleton()->get_locale());
	}

	fallback = GLOBAL_DEF("internationalization/locale/fallback", "en");
	pseudolocalization_enabled = GLOBAL_DEF("internationalization/pseudolocalization/use_pseudolocalization", false);
	pseudolocalization_accents_enabled = GLOBAL_DEF("internationalization/pseudolocalization/replace_with_accents", true);
	pseudolocalization_double_vowels_enabled = GLOBAL_DEF("internationalization/pseudolocalization/double_vowels", false);
	pseudolocalization_fake_bidi_enabled = GLOBAL_DEF("internationalization/pseudolocalization/fake_bidi", false);
	pseudolocalization_override_enabled = GLOBAL_DEF("internationalization/pseudolocalization/override", false);
	expansion_ratio = GLOBAL_DEF("internationalization/pseudolocalization/expansion_ratio", 0.0);
	pseudolocalization_prefix = GLOBAL_DEF("internationalization/pseudolocalization/prefix", "[");
	pseudolocalization_suffix = GLOBAL_DEF("internationalization/pseudolocalization/suffix", "]");
	pseudolocalization_skip_placeholders_enabled = GLOBAL_DEF("internationalization/pseudolocalization/skip_placeholders", true);

#ifdef TOOLS_ENABLED
	ProjectSettings::get_singleton()->set_custom_property_info(PropertyInfo(Variant::STRING, "internationalization/locale/fallback", PROPERTY_HINT_LOCALE_ID, ""));
#endif
}

void TranslationServer::set_tool_translation(const Ref<Translation> &p_translation) {
	tool_translation = p_translation;
}

Ref<Translation> TranslationServer::get_tool_translation() const {
	return tool_translation;
}

String TranslationServer::get_tool_locale() {
#ifdef TOOLS_ENABLED
	if (Engine::get_singleton()->is_editor_hint() || Engine::get_singleton()->is_project_manager_hint()) {
		if (TranslationServer::get_singleton()->get_tool_translation().is_valid()) {
			return tool_translation->get_locale();
		} else {
			return "en";
		}
	} else {
#else
	{
#endif
		// Look for best matching loaded translation.
		String best_locale = "en";
		int best_score = 0;

		for (const KeyValue<String, Vector<Ref<Translation>>> &E : translations) {
			String l = E.key;

			int score = compare_locales(locale, l);
			if (score > 0 && score >= best_score) {
				best_locale = l;
				best_score = score;
				if (score == 10) {
					break; // Exact match, skip the rest.
				}
			}
		}
		return best_locale;
	}
}

Variant TranslationServer::tool_translate(const StringName &p_message, const StringName &p_context) const {
	if (tool_translation.is_valid()) {
		StringName r = tool_translation->get_message(p_message);
		if (r) {
			return editor_pseudolocalization ? tool_pseudolocalize(r) : r;
		}
	}
	return editor_pseudolocalization ? tool_pseudolocalize(p_message) : p_message;
}

Variant TranslationServer::tool_translate_plural(const StringName &p_message, const StringName &p_message_plural, int p_n, const StringName &p_context) const {
	if (tool_translation.is_valid()) {
		StringName r = tool_translation->get_plural_message(p_message, p_message_plural, p_n);
		if (r) {
			return r;
		}
	}

	if (p_n == 1) {
		return p_message;
	}
	return p_message_plural;
}

void TranslationServer::set_doc_translation(const Ref<Translation> &p_translation) {
	doc_translation = p_translation;
}

Variant TranslationServer::doc_translate(const StringName &p_message, const StringName &p_context) const {
	if (doc_translation.is_valid()) {
		StringName r = doc_translation->get_message(p_message);
		if (r) {
			return r;
		}
	}
	return p_message;
}

Variant TranslationServer::doc_translate_plural(const StringName &p_message, const StringName &p_message_plural, int p_n, const StringName &p_context) const {
	if (doc_translation.is_valid()) {
		StringName r = doc_translation->get_plural_message(p_message, p_message_plural, p_n);
		if (r) {
			return r;
		}
	}

	if (p_n == 1) {
		return p_message;
	}
	return p_message_plural;
}

void TranslationServer::set_property_translation(const Ref<Translation> &p_translation) {
	property_translation = p_translation;
}

Variant TranslationServer::property_translate(const StringName &p_message, const StringName &p_context) const {
	if (property_translation.is_valid()) {
		StringName r = property_translation->get_message(p_message);
		if (r) {
			return r;
		}
	}
	return p_message;
}

bool TranslationServer::is_pseudolocalization_enabled() const {
	return pseudolocalization_enabled;
}

void TranslationServer::set_pseudolocalization_enabled(bool p_enabled) {
	pseudolocalization_enabled = p_enabled;

	if (OS::get_singleton()->get_main_loop()) {
		OS::get_singleton()->get_main_loop()->notification(MainLoop::NOTIFICATION_TRANSLATION_CHANGED);
	}
	ResourceLoader::reload_translation_remaps();
}

void TranslationServer::set_editor_pseudolocalization(bool p_enabled) {
	editor_pseudolocalization = p_enabled;
}

void TranslationServer::reload_pseudolocalization() {
	pseudolocalization_accents_enabled = GLOBAL_GET("internationalization/pseudolocalization/replace_with_accents");
	pseudolocalization_double_vowels_enabled = GLOBAL_GET("internationalization/pseudolocalization/double_vowels");
	pseudolocalization_fake_bidi_enabled = GLOBAL_GET("internationalization/pseudolocalization/fake_bidi");
	pseudolocalization_override_enabled = GLOBAL_GET("internationalization/pseudolocalization/override");
	expansion_ratio = GLOBAL_GET("internationalization/pseudolocalization/expansion_ratio");
	pseudolocalization_prefix = GLOBAL_GET("internationalization/pseudolocalization/prefix");
	pseudolocalization_suffix = GLOBAL_GET("internationalization/pseudolocalization/suffix");
	pseudolocalization_skip_placeholders_enabled = GLOBAL_GET("internationalization/pseudolocalization/skip_placeholders");

	if (OS::get_singleton()->get_main_loop()) {
		OS::get_singleton()->get_main_loop()->notification(MainLoop::NOTIFICATION_TRANSLATION_CHANGED);
	}
	ResourceLoader::reload_translation_remaps();
}

StringName TranslationServer::pseudolocalize(const StringName &p_message) const {
	String message = p_message;
	int length = message.length();
	if (pseudolocalization_override_enabled) {
		message = get_override_string(message);
	}

	if (pseudolocalization_double_vowels_enabled) {
		message = double_vowels(message);
	}

	if (pseudolocalization_accents_enabled) {
		message = replace_with_accented_string(message);
	}

	if (pseudolocalization_fake_bidi_enabled) {
		message = wrap_with_fakebidi_characters(message);
	}

	StringName res = add_padding(message, length);
	return res;
}

StringName TranslationServer::tool_pseudolocalize(const StringName &p_message) const {
	String message = p_message;
	message = double_vowels(message);
	message = replace_with_accented_string(message);
	StringName res = "[!!! " + message + " !!!]";
	return res;
}

String TranslationServer::get_override_string(String &p_message) const {
	String res;
	for (int i = 0; i < p_message.size(); i++) {
		if (pseudolocalization_skip_placeholders_enabled && is_placeholder(p_message, i)) {
			res += p_message[i];
			res += p_message[i + 1];
			i++;
			continue;
		}
		res += '*';
	}
	return res;
}

String TranslationServer::double_vowels(String &p_message) const {
	String res;
	for (int i = 0; i < p_message.size(); i++) {
		if (pseudolocalization_skip_placeholders_enabled && is_placeholder(p_message, i)) {
			res += p_message[i];
			res += p_message[i + 1];
			i++;
			continue;
		}
		res += p_message[i];
		if (p_message[i] == 'a' || p_message[i] == 'e' || p_message[i] == 'i' || p_message[i] == 'o' || p_message[i] == 'u' ||
				p_message[i] == 'A' || p_message[i] == 'E' || p_message[i] == 'I' || p_message[i] == 'O' || p_message[i] == 'U') {
			res += p_message[i];
		}
	}
	return res;
};

String TranslationServer::replace_with_accented_string(String &p_message) const {
	String res;
	for (int i = 0; i < p_message.size(); i++) {
		if (pseudolocalization_skip_placeholders_enabled && is_placeholder(p_message, i)) {
			res += p_message[i];
			res += p_message[i + 1];
			i++;
			continue;
		}
		const char32_t *accented = get_accented_version(p_message[i]);
		if (accented) {
			res += accented;
		} else {
			res += p_message[i];
		}
	}
	return res;
}

String TranslationServer::wrap_with_fakebidi_characters(String &p_message) const {
	String res;
	char32_t fakebidiprefix = U'\u202e';
	char32_t fakebidisuffix = U'\u202c';
	res += fakebidiprefix;
	// The fake bidi unicode gets popped at every newline so pushing it back at every newline.
	for (int i = 0; i < p_message.size(); i++) {
		if (p_message[i] == '\n') {
			res += fakebidisuffix;
			res += p_message[i];
			res += fakebidiprefix;
		} else if (pseudolocalization_skip_placeholders_enabled && is_placeholder(p_message, i)) {
			res += fakebidisuffix;
			res += p_message[i];
			res += p_message[i + 1];
			res += fakebidiprefix;
			i++;
		} else {
			res += p_message[i];
		}
	}
	res += fakebidisuffix;
	return res;
}

String TranslationServer::add_padding(const String &p_message, int p_length) const {
	String res;
	String prefix = pseudolocalization_prefix;
	String suffix;
	for (int i = 0; i < p_length * expansion_ratio / 2; i++) {
		prefix += "_";
		suffix += "_";
	}
	suffix += pseudolocalization_suffix;
	res += prefix;
	res += p_message;
	res += suffix;
	return res;
}

const char32_t *TranslationServer::get_accented_version(char32_t p_character) const {
	if (!is_ascii_char(p_character)) {
		return nullptr;
	}

	for (unsigned int i = 0; i < sizeof(_character_to_accented) / sizeof(_character_to_accented[0]); i++) {
		if (_character_to_accented[i].character == p_character) {
			return _character_to_accented[i].accented_character;
		}
	}

	return nullptr;
}

bool TranslationServer::is_placeholder(String &p_message, int p_index) const {
	return p_index < p_message.size() - 1 && p_message[p_index] == '%' &&
			(p_message[p_index + 1] == 's' || p_message[p_index + 1] == 'c' || p_message[p_index + 1] == 'd' ||
					p_message[p_index + 1] == 'o' || p_message[p_index + 1] == 'x' || p_message[p_index + 1] == 'X' || p_message[p_index + 1] == 'f');
}

void TranslationServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_locale", "locale"), &TranslationServer::set_locale);
	ClassDB::bind_method(D_METHOD("get_locale"), &TranslationServer::get_locale);
	ClassDB::bind_method(D_METHOD("get_tool_locale"), &TranslationServer::get_tool_locale);

	ClassDB::bind_method(D_METHOD("compare_locales", "locale_a", "locale_b"), &TranslationServer::compare_locales);
	ClassDB::bind_method(D_METHOD("standardize_locale", "locale"), &TranslationServer::standardize_locale);

	ClassDB::bind_method(D_METHOD("get_all_languages"), &TranslationServer::get_all_languages);
	ClassDB::bind_method(D_METHOD("get_language_name", "language"), &TranslationServer::get_language_name);

	ClassDB::bind_method(D_METHOD("get_all_scripts"), &TranslationServer::get_all_scripts);
	ClassDB::bind_method(D_METHOD("get_script_name", "script"), &TranslationServer::get_script_name);

	ClassDB::bind_method(D_METHOD("get_all_countries"), &TranslationServer::get_all_countries);
	ClassDB::bind_method(D_METHOD("get_country_name", "country"), &TranslationServer::get_country_name);

	ClassDB::bind_method(D_METHOD("get_locale_name", "locale"), &TranslationServer::get_locale_name);

	ClassDB::bind_method(D_METHOD("translate", "key", "context"), &TranslationServer::translate, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("translate_plural", "message", "plural_message", "n", "context"), &TranslationServer::translate_plural, DEFVAL(""));

	ClassDB::bind_method(D_METHOD("add_translation", "translation"), &TranslationServer::add_translation);
	ClassDB::bind_method(D_METHOD("remove_translation", "translation"), &TranslationServer::remove_translation);

	ClassDB::bind_method(D_METHOD("get_translation_objects", "locale"), &TranslationServer::get_translation_objects);
	ClassDB::bind_method(D_METHOD("get_translation_object_count", "locale"), &TranslationServer::get_translation_object_count);
	ClassDB::bind_method(D_METHOD("get_translation_object_at", "locale", "index"), &TranslationServer::get_translation_object_at);

	ClassDB::bind_method(D_METHOD("clear"), &TranslationServer::clear);
	ClassDB::bind_method(D_METHOD("get_loaded_locales"), &TranslationServer::get_loaded_locales);
	ClassDB::bind_method(D_METHOD("load_translations"), &TranslationServer::load_translations);

	ClassDB::bind_method(D_METHOD("is_pseudolocalization_enabled"), &TranslationServer::is_pseudolocalization_enabled);
	ClassDB::bind_method(D_METHOD("set_pseudolocalization_enabled", "enabled"), &TranslationServer::set_pseudolocalization_enabled);
	ClassDB::bind_method(D_METHOD("reload_pseudolocalization"), &TranslationServer::reload_pseudolocalization);
	ClassDB::bind_method(D_METHOD("pseudolocalize", "message"), &TranslationServer::pseudolocalize);
	ADD_PROPERTY(PropertyInfo(Variant::Type::BOOL, "pseudolocalization_enabled"), "set_pseudolocalization_enabled", "is_pseudolocalization_enabled");
}

void TranslationServer::load_translations() {
	clear();
	_load_translations("internationalization/locale/translations"); //all
	_load_translations("internationalization/locale/translations_" + locale.substr(0, 2));

	if (locale.substr(0, 2) != locale) {
		_load_translations("internationalization/locale/translations_" + locale);
	}
}

TranslationServer::TranslationServer() {
	singleton = this;
	init_locale_info();
}
