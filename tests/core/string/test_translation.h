/**************************************************************************/
/*  test_translation.h                                                    */
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

#ifndef TEST_TRANSLATION_H
#define TEST_TRANSLATION_H

#include "core/string/optimized_translation.h"
#include "core/string/translation.h"

#ifdef TOOLS_ENABLED
#include "editor/import/resource_importer_csv_translation.h"
#endif

#include "tests/test_macros.h"
#include "tests/test_utils.h"

namespace TestTranslation {

TEST_CASE("[Translation] Messages") {
	Ref<Translation> translation = memnew(Translation);
	translation->set_locale("fr");
	translation->add_message("Hello", "Bonjour");
	CHECK(translation->get_message("Hello") == "Bonjour");

	translation->erase_message("Hello");
	// The message no longer exists, so it returns an empty string instead.
	CHECK(translation->get_message("Hello") == Variant());

	List<StringName> messages;
	translation->get_message_list(&messages);
	CHECK(translation->get_message_count() == 0);
	CHECK(messages.size() == 0);

	translation->add_message("Hello2", "Bonjour2");
	translation->add_message("Hello3", "Bonjour3");
	messages.clear();
	translation->get_message_list(&messages);
	CHECK(translation->get_message_count() == 2);
	CHECK(messages.size() == 2);
	// Messages are stored in a Map, don't assume ordering.
	CHECK(messages.find("Hello2"));
	CHECK(messages.find("Hello3"));
}

#ifdef TOOLS_ENABLED
TEST_CASE("[TranslationCSV] CSV import") {
	Ref<ResourceImporterCSVTranslation> import_csv_translation = memnew(ResourceImporterCSVTranslation);

	HashMap<StringName, Variant> options;
	options["compress"] = false;
	options["delimiter"] = 0;

	List<String> gen_files;

	Error result = import_csv_translation->import(TestUtils::get_data_path("translations.csv"),
			"", options, nullptr, &gen_files);
	CHECK(result == OK);
	CHECK(gen_files.size() == 4);

	TranslationServer *ts = TranslationServer::get_singleton();

	for (const String &file : gen_files) {
		Ref<Translation> translation = ResourceLoader::load(file);
		CHECK(translation.is_valid());
		ts->add_translation(translation);
	}

	ts->set_locale("en");

	// `tr` can be called on any Object, we reuse TranslationServer for convenience.
	CHECK(ts->tr("GOOD_MORNING") == "Good Morning");
	CHECK(ts->tr("GOOD_EVENING") == "Good Evening");

	ts->set_locale("de");

	CHECK(ts->tr("GOOD_MORNING") == "Guten Morgen");
	CHECK(ts->tr("GOOD_EVENING") == "Good Evening"); // Left blank in CSV, should source from 'en'.

	ts->set_locale("ja");

	CHECK(ts->tr("GOOD_MORNING") == String::utf8("おはよう"));
	CHECK(ts->tr("GOOD_EVENING") == String::utf8("こんばんは"));

	/* FIXME: This passes, but triggers a chain reaction that makes test_viewport
	 * and test_text_edit explode in a billion glittery Unicode particles.
	ts->set_locale("fa");

	CHECK(ts->tr("GOOD_MORNING") == String::utf8("صبح بخیر"));
	CHECK(ts->tr("GOOD_EVENING") == String::utf8("عصر بخیر"));
	*/
}
#endif // TOOLS_ENABLED

} // namespace TestTranslation

#endif // TEST_TRANSLATION_H
